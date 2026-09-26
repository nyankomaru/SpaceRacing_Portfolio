// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerChara.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MyCalcu.h"
#include "MyCamera.h"
#include "MyGameInstance.h"
#include "MyWorldSubsystem.h"
#include "Planet.h"

// ==================================================
// Constructor / Initialize
// ==================================================

APlayerChara::APlayerChara()
	: m_Velocity(0.0f)
	, m_ReturnCourseVelo(0.0f)
	, m_Rot(0.0f)
	, m_CameraRot(0.0f)
	, m_CameraRotInput(0.0f)
	, m_ForwardInput(0.0f)
	, m_PreForwardInput(0.0f)
	, m_PreRotIn(0.0f)
	, m_NowRotSpeed(0.0f)
	, m_ChangeCtrl(0.0f)
	, m_Speed(0.0f)
	, m_ForwardInputTime(0.0f)
	, m_StrongFOVTimer(0.0f)
	, m_ReachMaxRotSpeed(1.0f)
	, m_bCollisiON(false)
	, m_bCamConChange(false)
	, m_bAutoRot(false)
	, m_bReturnCource(false)
{
	PrimaryActorTick.bCanEverTick = true;

	// -------------------------
	// Mesh / Collision
	// -------------------------
	m_pMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("m_pMesh"));
	if (m_pMesh)
	{
		// ルートに設定
		RootComponent = m_pMesh;

		// メッシュ側でオーバーラップ判定を受ける
		m_pMesh->SetGenerateOverlapEvents(true);
		m_pMesh->OnComponentBeginOverlap.AddDynamic(this, &APlayerChara::OnOverlapBegin);
		m_pMesh->OnComponentEndOverlap.AddDynamic(this, &APlayerChara::OnOverlapEnd);

		// 当たり判定設定
		m_pMesh->SetCollisionProfileName(TEXT("MyBlockDynamic"));
		m_pMesh->SetEnableGravity(false);
	}

	// -------------------------
	// Camera
	// -------------------------
	m_pSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("m_pSpring"));
	if (m_pSpring)
	{
		m_pSpring->SetupAttachment(RootComponent);
	}

	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("m_pCamera"));
	if (m_pCamera)
	{
		m_pCamera->SetupAttachment(m_pSpring);
	}

	// -------------------------
	// Movement
	// -------------------------
	m_pMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("m_pMovement"));
	if (m_pMovement)
	{
		// 初期設定は必要に応じて追加
	}

	// -------------------------
	// Audio（主担当）
	// -------------------------
	EngineAudioLow = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioLow"));
	if (EngineAudioLow)
	{
		EngineAudioLow->SetupAttachment(RootComponent);
		EngineAudioLow->bAutoActivate = false;
		EngineAudioLow->bIsUISound = false;
	}

	EngineAudioHigh = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioHigh"));
	if (EngineAudioHigh)
	{
		EngineAudioHigh->SetupAttachment(RootComponent);
		EngineAudioHigh->bAutoActivate = false;
		EngineAudioHigh->bIsUISound = false;
	}

	RotateAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RotateAudio"));
	if (RotateAudio)
	{
		RotateAudio->SetupAttachment(RootComponent);
		RotateAudio->bAutoActivate = false;
		RotateAudio->bIsUISound = false;
	}
}

// ==================================================
// Engine Override
// ==================================================

void APlayerChara::BeginPlay()
{
	Super::BeginPlay();

	// GameInstanceへ登録
	if (UMyGameInstance* GameInstance = GetGameInstance<UMyGameInstance>())
	{
		GameInstance->SetPlayer(this);
		m_pSpline = GameInstance->GetCourseSpline();
	}

	// UI進行度用の初期化
	if (m_pSpline)
	{
		m_SplineLen = m_pSpline->GetSplineLength();
		m_CourseS = 0.0f;
		m_CourseSPrev = 0.0f;
		m_CourseSBest = 0.0f;
		m_CourseSDisplay = 0.0f;
		m_bReverse = false;
	}

	// Audioループ開始（主担当）
	if (EngineAudioLow && EngineLowLoopSound)
	{
		EngineAudioLow->SetSound(EngineLowLoopSound);
		EngineAudioLow->Play();
	}

	if (EngineAudioHigh && EngineHighLoopSound)
	{
		EngineAudioHigh->SetSound(EngineHighLoopSound);
		EngineAudioHigh->Play();
	}

	if (RotateAudio && RotateLoopSound)
	{
		RotateAudio->SetSound(RotateLoopSound);
		RotateAudio->Play();
	}
}

void APlayerChara::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// -------------------------
	// Audio用入力キャッシュ（主担当）
	// -------------------------
	m_ThrustInput01ThisFrame = FMath::Clamp(FMath::Abs(m_ForwardInput), 0.0f, 1.0f);

	// -------------------------
	// 基本更新
	// -------------------------
	UpdateRotation(DeltaTime);
	UpdateMove(DeltaTime);
	UpdateCamera(DeltaTime);
	UpdateSocket();

	// -------------------------
	// UI進行度更新（主担当）
	// -------------------------
	UpdateCourseProgress(DeltaTime);

	// -------------------------
	// Audio更新（主担当）
	// -------------------------
	UpdateEngineAudio(DeltaTime);
	UpdateRotateAudio(DeltaTime);

	// -------------------------
	// フレーム終端処理
	// -------------------------
	m_RotateInput01 = 0.0f;
	ValueReset();
}

void APlayerChara::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// -------------------------
	// Forward / Back
	// -------------------------
	InputComponent->BindAxis("RightTrigger", this, &APlayerChara::MoveForward);
	InputComponent->BindAxis("LeftTrigger", this, &APlayerChara::MoveForward);
	InputComponent->BindAxis("Space", this, &APlayerChara::MoveForward);
	InputComponent->BindAxis("Right_Ctrl", this, &APlayerChara::MoveForward);

	// -------------------------
	// Rotation
	// -------------------------
	InputComponent->BindAxis("LeftStickY", this, &APlayerChara::RotPitch);
	InputComponent->BindAxis("W_Key", this, &APlayerChara::RotPitch);
	InputComponent->BindAxis("S_Key", this, &APlayerChara::RotPitch);

	InputComponent->BindAxis("LeftStickX", this, &APlayerChara::RotYaw);
	InputComponent->BindAxis("A_Key", this, &APlayerChara::RotYaw);
	InputComponent->BindAxis("D_Key", this, &APlayerChara::RotYaw);

	InputComponent->BindAxis("RightShoulder", this, &APlayerChara::RotRoll);
	InputComponent->BindAxis("LeftShoulder", this, &APlayerChara::RotRoll);
	InputComponent->BindAxis("E_Key", this, &APlayerChara::RotRoll);
	InputComponent->BindAxis("Q_Key", this, &APlayerChara::RotRoll);

	// -------------------------
	// Camera Rotation
	// -------------------------
	InputComponent->BindAxis("RightStickX", this, &APlayerChara::CameraRotPitch);
	InputComponent->BindAxis("Right_Key", this, &APlayerChara::CameraRotPitch);
	InputComponent->BindAxis("Left_Key", this, &APlayerChara::CameraRotPitch);

	InputComponent->BindAxis("RightStickY", this, &APlayerChara::CameraRotYaw);
	InputComponent->BindAxis("Up_Key", this, &APlayerChara::CameraRotYaw);
	InputComponent->BindAxis("Down_Key", this, &APlayerChara::CameraRotYaw);

	// -------------------------
	// Option / Debug Style Input
	// -------------------------
	InputComponent->BindAxis("Left_Ctrl", this, &APlayerChara::ChangeCtrl);
	InputComponent->BindAction("O_Key", EInputEvent::IE_Pressed, this, &APlayerChara::ChangeCollision);
	InputComponent->BindAction("V_Key", EInputEvent::IE_Pressed, this, &APlayerChara::ChangeCamCon);
	InputComponent->BindAction("R_Key", EInputEvent::IE_Pressed, this, &APlayerChara::ChangeAutoRot);
}

// ==================================================
// Overlap
// ==================================================

void APlayerChara::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherComp)
	{
		return;
	}

	// 重力判定オブジェクトなら追加
	if (OtherComp->ComponentHasTag(TEXT("Gravity")))
	{
		m_pPlanets.Add(Cast<APlanet>(OtherActor));
	}
}

void APlayerChara::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (!OtherComp)
	{
		return;
	}

	// 重力判定オブジェクトなら除外
	if (OtherComp->ComponentHasTag(TEXT("Gravity")))
	{
		m_pPlanets.Remove(Cast<APlanet>(OtherActor));
	}
}

// ==================================================
// Public Utility
// ==================================================

void APlayerChara::AddSocket(AActor* _p, FVector _Pos, bool _bRot)
{
	if (!_p)
	{
		return;
	}

	m_pSocket.Add(_p);
	m_SocketPos.Add(_Pos);
	m_bSocketRot.Add(_bRot);
}

void APlayerChara::RemoveSocket(AActor* _p)
{
	if (_p)
	{
		// 既存コードでは未実装
	}
}

void APlayerChara::ChangeCamera(USpringArmComponent* _pSpring, UCameraComponent* _pCamera)
{
	if (_pSpring && _pCamera)
	{
		m_pSpring = _pSpring;
		m_pCamera = _pCamera;
	}
}

FRotator APlayerChara::GetUpRotator()
{
	return UKismetMathLibrary::FindLookAtRotation(FVector(0.0f), GetActorUpVector());
}

int APlayerChara::GetGraNum()
{
	return m_pPlanets.Num();
}

float APlayerChara::GetSpeed()
{
	return m_pMovement->Velocity.Length();
}

float APlayerChara::GetMaxSpeed()
{
	return m_pMovement->MaxSpeed;
}

float APlayerChara::GetForwardInput() const
{
	return m_ForwardInput;
}

float APlayerChara::GetForwardInputTime() const
{
	return m_ForwardInputTime;
}

void APlayerChara::SetStart(FVector _Loc, FRotator _Rot, float _Speed)
{
	SetActorRotation(_Rot);
	SetActorLocation(_Loc);
	m_pMovement->Velocity = m_pMesh->GetForwardVector() * _Speed;
}

void APlayerChara::SubSpeed(float _Rate)
{
	m_pMovement->Velocity *= _Rate;
}

// ==================================================
// Update : Rotation
// ==================================================

void APlayerChara::UpdateRotation(float DeltaTime)
{
	// 回転入力がある時
	if (!m_Rot.IsZero())
	{
		for (int i = 0; i < 3; ++i)
		{
			if (*(&m_Rot.Pitch + i) != 0.0f)
			{
				// 入力方向が変わったら回転速度をリセット
				if (FMath::Sign(*(&m_Rot.Pitch + i)) != FMath::Sign(*(&m_PreRotIn.Pitch + i)))
				{
					*(&m_NowRotSpeed.Pitch + i) = 0.0f;
				}

				*(&m_NowRotSpeed.Pitch + i) +=
					FMath::Sign(*(&m_Rot.Pitch + i)) * DeltaTime / m_ReachMaxRotSpeed;
			}
			else
			{
				*(&m_NowRotSpeed.Pitch + i) -=
					FMath::Sign(*(&m_NowRotSpeed.Pitch + i)) * DeltaTime / m_ReachMaxRotSpeed;
			}

			*(&m_NowRotSpeed.Pitch + i) =
				FMath::Clamp(FMath::Abs(*(&m_NowRotSpeed.Pitch + i)), 0.0f, 1.0f) *
				FMath::Sign(*(&m_NowRotSpeed.Pitch + i));
		}

		// 重心に中心を合わせて回転
		AddActorLocalOffset(m_RotPivot);

		FVector VecDire[3](GetActorForwardVector(), GetActorRightVector(), GetActorUpVector());
		FVector RotAxis[3](m_pSpring->GetRightVector(), m_pSpring->GetUpVector(), m_pSpring->GetForwardVector());

		for (int i = 0; i < 3; ++i)
		{
			for (int n = 0; n < 3; ++n)
			{
				VecDire[i] = VecDire[i].RotateAngleAxis(
					(*(&m_Rot.Pitch + n)) * FMath::Abs(*(&m_NowRotSpeed.Pitch + n)) * DeltaTime * m_MaxRotSpeed,
					RotAxis[n]
				);
			}
		}

		SetActorRotation(UKismetMathLibrary::MakeRotationFromAxes(VecDire[0], VecDire[1], VecDire[2]));

		// 回転中心を戻す
		AddActorLocalOffset(-m_RotPivot);
	}
	else
	{
		// 回転入力がない時は回転速度を減衰
		for (int i = 0; i < 3; ++i)
		{
			*(&m_NowRotSpeed.Pitch + i) -=
				FMath::Sign(*(&m_NowRotSpeed.Pitch + i)) * DeltaTime / m_ReachMaxRotSpeed;
		}

		// コース外、または前進入力なしの時は進行方向へ戻す
		if (m_ForwardInput == 0.0f || m_bReturnCource)
		{
			SetActorRotation(
				UKismetMathLibrary::RInterpTo(
					GetActorRotation(),
					m_pMovement->Velocity.Rotation(),
					DeltaTime,
					m_ReturnRotSpeed *
					(1.0f - (m_NowRotSpeed.Pitch + m_NowRotSpeed.Yaw + m_NowRotSpeed.Roll) / 3.0f)
				)
			);
		}
	}

	m_PreRotIn = m_Rot;
}

// ==================================================
// Update : Move
// ==================================================

void APlayerChara::UpdateMove(float DeltaTime)
{
	FVector Loc(GetActorLocation());
	FVector AddMoveDire(0.0f);
	FVector AddReturn(0.0f);
	float CameraLagDistance(m_pSpring->CameraLagMaxDistance);

	// -------------------------
	// Forward Input
	// -------------------------
	if (FMath::Abs(m_ForwardInput) != 0.0f)
	{
		if (m_ForwardInput >= 0.0f)
		{
			AddMoveDire += m_pMesh->GetForwardVector() * m_ForwardInput * m_ForwardSpeed;
		}
		else
		{
			AddMoveDire += m_pMovement->Velocity * -1.0f * 0.8f;
		}

		m_ForwardInputTime += DeltaTime;
	}
	else
	{
		m_ForwardInputTime -= DeltaTime;
	}

	// 入力開始直後は少し強めにFOV演出
	if (m_PreForwardInput != m_ForwardInput && m_PreForwardInput == 0.0f && m_StrongFOVTimer == 0.0f)
	{
		m_StrongFOVTimer = 0.1f;
	}
	m_PreForwardInput = m_ForwardInput;

	// カメララグ距離制限
	m_pSpring->CameraLagMaxDistance = FMath::Clamp(CameraLagDistance, 0.1f, m_CameraLagMaxDistance);

	// 入力継続時間を0～1に収める
	m_ForwardInputTime = FMath::Clamp(m_ForwardInputTime, 0.0f, 1.0f);

	// -------------------------
	// Gravity
	// -------------------------
	if (m_pPlanets.Num() != 0)
	{
		FVector GravityVec(0.0f);
		int NearID(0);

		for (int i = 0; i < m_pPlanets.Num(); ++i)
		{
			FVector PlanetDire(m_pPlanets[i]->GetActorLocation() - Loc);
			float Distance(PlanetDire.Length() - m_pPlanets[i]->GetRadius());

			float Gravity(
				m_pPlanets[i]->GetGravity() *
				(1.0f - Distance / (m_pPlanets[i]->GetGradius() - m_pPlanets[i]->GetRadius()))
			);

			DrawDebugLine(GetWorld(), Loc, Loc + PlanetDire, FColor::Red);

			GravityVec += PlanetDire.GetSafeNormal() * Gravity;

			if ((m_pPlanets[NearID]->GetActorLocation() - Loc).Length() > Distance)
			{
				NearID = i;
			}
		}

		AddMoveDire += GravityVec;
	}

	// 加速度分を蓄積
	m_Velocity += AddMoveDire * DeltaTime;

	// -------------------------
	// Return To Course
	// -------------------------
	if (m_pSpline && m_ReturnCourseLen != 0.0f)
	{
		FVector NearCourseLoc(
			m_pSpline->FindLocationClosestToWorldLocation(Loc, ESplineCoordinateSpace::World)
		);
		FVector NearCourseVec(NearCourseLoc - Loc);
		float NearCourseLen(NearCourseVec.Length());

		m_bReturnCource = false;

		if (NearCourseLen >= m_ReturnCourseLen)
		{
			m_pMovement->Velocity =
				m_pSpline->FindDirectionClosestToWorldLocation(Loc, ESplineCoordinateSpace::World) *
				(m_pMovement->Velocity.Length() * M_CourseOutRate);

			m_pMovement->Velocity += NearCourseVec * DeltaTime;
			m_bReturnCource = true;
		}

		DrawDebugLine(GetWorld(), Loc, Loc + NearCourseVec, FColor::Orange);
	}

	// -------------------------
	// Final Movement Apply
	// -------------------------
	m_pMovement->Acceleration = (AddMoveDire + AddReturn).Length();
	AddMovementInput((AddMoveDire + AddReturn).GetSafeNormal());

	Loc = GetActorLocation();
	m_Speed = (Loc - m_PreLoc).Length() / DeltaTime;
	m_PreLoc = Loc;

	DrawDebugLine(GetWorld(), Loc, Loc + m_pMesh->GetForwardVector() * m_ForwardSpeed, FColor::Yellow);
	DrawDebugLine(GetWorld(), Loc, Loc + m_pMovement->Velocity, FColor::Green);
}

// ==================================================
// Update : Camera
// ==================================================

void APlayerChara::UpdateCamera(float DeltaTime)
{
	UpdateCameraRot(DeltaTime);
	UpdateCameraMove(DeltaTime);
	UpdateCameraFOV(DeltaTime);
}

void APlayerChara::UpdateCameraRot(float DeltaTime)
{
	// カメラ入力がある時は回転
	if (!m_CameraRotInput.IsZero())
	{
		if (m_ChangeCtrl > 0.0f)
		{
			m_pCamera->AddLocalRotation(m_CameraRotInput * DeltaTime * m_CameraRotSpeed);
			m_ChangeCtrl = 0.0f;
		}
		else
		{
			m_CameraRotInput *= DeltaTime * m_CameraRotSpeed;
			m_CameraRotInput.Roll = 0.0f;
			m_pSpring->AddRelativeRotation(m_CameraRotInput);
		}

		m_CameraRotInput = FRotator(0.0f, 0.0f, 0.0f);
	}
	else
	{
		m_pCamera->SetRelativeRotation(
			UKismetMathLibrary::RInterpTo(
				m_pCamera->GetRelativeRotation(),
				m_DefaCameraRot,
				DeltaTime,
				m_CameraReturnRotSpeed
			)
		);
	}

	// 進行方向と機体前方の中間方向へ補間
	{
		FRotator MidRot((m_pMovement->Velocity.GetSafeNormal() + m_pMesh->GetForwardVector()).Rotation());

		m_pSpring->SetWorldRotation(
			UKismetMathLibrary::RInterpTo(
				m_pSpring->GetComponentRotation(),
				MidRot,
				DeltaTime,
				m_CameraReturnRotSpeed
			)
		);
	}
}

void APlayerChara::UpdateCameraMove(float DeltaTime)
{
	FVector DefaPos(GetActorLocation() + m_DefaAddSpringPos);
	FVector LagDire(m_pMesh->GetForwardVector() * m_CameraLagMaxDistance);
	FVector CamDire[2](m_pSpring->GetRightVector(), m_pSpring->GetUpVector());

	FVector2D XY(0.0f);
	XY.X = FVector::DotProduct(CamDire[0], LagDire);
	XY.Y = FVector::DotProduct(CamDire[1], LagDire * 0.618f);

	if (m_ForwardInput == 0.0f)
	{
		m_pSpring->SetWorldLocation(
			FMath::VInterpTo(
				m_pSpring->GetComponentLocation(),
				DefaPos,
				DeltaTime,
				m_CameraLagDistanceSpeed
			)
		);
	}
	else
	{
		m_pSpring->SetWorldLocation(
			FMath::VInterpTo(
				m_pSpring->GetComponentLocation(),
				DefaPos + CamDire[0] * XY.X + CamDire[1] * XY.Y,
				DeltaTime,
				m_CameraLagDistanceSpeed
			)
		);
	}

	FVector NowDisVec((m_pSpring->GetComponentLocation() - (GetActorLocation() + m_DefaAddSpringPos)));
	FVector2D NowXYDis(
		FMath::Abs(FVector::DotProduct(CamDire[0], NowDisVec)),
		FMath::Abs(FVector::DotProduct(CamDire[1], NowDisVec))
	);

	if (NowXYDis[0] > m_CameraLagMaxDistance || NowXYDis[1] > m_CameraLagMaxDistance * 0.618f)
	{
		m_pSpring->SetWorldLocation(
			DefaPos
			+ CamDire[0] * ((NowXYDis[0] > m_CameraLagMaxDistance) ? XY.X : NowXYDis[0])
			+ CamDire[1] * ((NowXYDis[1] > m_CameraLagMaxDistance * 0.618f) ? XY.Y : NowXYDis[1])
		);
	}
}

void APlayerChara::UpdateCameraFOV(float DeltaTime)
{
	float NowAddFOV(m_pCamera->FieldOfView - m_DefaFOV);
	float Rate(m_pMovement->Velocity.Length() / (m_pMovement->MaxSpeed));

	if (m_StrongFOVTimer == 0.0f)
	{
		NowAddFOV = MyCalcu::ToValueF(NowAddFOV, Rate * m_MaxAddFOV, m_DefaAddFOVSpeed, DeltaTime);
	}
	else
	{
		NowAddFOV = MyCalcu::ToValueF(
			NowAddFOV,
			Rate * m_MaxAddFOV + m_SpeedUpFOV,
			m_SpeedUpAddFOVSpeed,
			DeltaTime
		);
		m_StrongFOVTimer = MyCalcu::ToValueF(m_StrongFOVTimer, 0.0f, 1.0f, DeltaTime);
	}

	m_pCamera->FieldOfView =
		m_DefaFOV + MyCalcu::Clamp(NowAddFOV, 0.0f, m_MaxAddFOV + m_SpeedUpFOV);
}

// ==================================================
// Update : Socket
// ==================================================

void APlayerChara::UpdateSocket()
{
	for (int i = 0; i < m_pSocket.Num(); ++i)
	{
		FVector SPos(m_SocketPos[i]);

		m_pSocket[i]->SetActorLocation(
			GetActorLocation()
			+ GetActorRightVector() * SPos.X
			+ GetActorForwardVector() * -1.0f * SPos.Y
			+ GetActorUpVector() * SPos.Z
		);

		if (m_bSocketRot[i])
		{
			m_pSocket[i]->SetActorRotation(GetActorRotation());
		}
	}
}

// ==================================================
// Input
// ==================================================

void APlayerChara::ValueReset()
{
	m_ForwardInput = 0.0f;
	m_Rot = FRotator(0.0f, 0.0f, 0.0f);
}

void APlayerChara::MoveForward(float _value)
{
	if (_value != 0.0f)
	{
		m_ForwardInput = _value;
	}
}

void APlayerChara::Deceleration(float _value)
{
	if (_value != 0.0f)
	{
		// 既存コードでは未実装
	}
}

void APlayerChara::RotPitch(float _value)
{
	if (_value != 0.0f)
	{
		m_Rot.Pitch = _value;
		m_RotateInput01 = FMath::Max(m_RotateInput01, FMath::Clamp(FMath::Abs(_value), 0.0f, 1.0f));
	}
}

void APlayerChara::RotYaw(float _value)
{
	if (_value != 0.0f)
	{
		m_Rot.Yaw = _value;
		m_RotateInput01 = FMath::Max(m_RotateInput01, FMath::Clamp(FMath::Abs(_value), 0.0f, 1.0f));
	}
}

void APlayerChara::RotRoll(float _value)
{
	if (_value != 0.0f)
	{
		m_Rot.Roll = _value;
		m_RotateInput01 = FMath::Max(m_RotateInput01, FMath::Clamp(FMath::Abs(_value), 0.0f, 1.0f));
	}
}

void APlayerChara::CameraRotYaw(float _value)
{
	if (_value != 0.0f)
	{
		m_CameraRotInput.Pitch = _value;
	}
}

void APlayerChara::CameraRotPitch(float _value)
{
	if (_value != 0.0f)
	{
		m_CameraRotInput.Yaw = _value;
	}
}

void APlayerChara::ChangeCtrl(float _value)
{
	m_ChangeCtrl = _value;
}

void APlayerChara::ChangeCollision()
{
	if (!m_bCollisiON)
	{
		m_pMesh->SetCollisionProfileName(TEXT("MyBlockDynamic"));
	}
	else
	{
		m_pMesh->SetCollisionProfileName(TEXT("AllIgnore"));
	}

	m_bCollisiON = !m_bCollisiON;
}

void APlayerChara::ChangeCamCon()
{
	m_bCamConChange = !m_bCamConChange;
}

void APlayerChara::ChangeAutoRot()
{
	m_bAutoRot = !m_bAutoRot;
}

// ==================================================
// Audio Update（主担当）
// ==================================================

void APlayerChara::UpdateEngineAudio(float DeltaTime)
{
	if (!EngineAudioLow && !EngineAudioHigh)
	{
		return;
	}

	// 推力入力を滑らかに反映
	EngineThrustSmoothed = FMath::FInterpTo(
		EngineThrustSmoothed,
		m_ThrustInput01ThisFrame,
		DeltaTime,
		EngineInterpSpeed
	);

	// Low Layer
	if (EngineAudioLow && EngineAudioLow->IsPlaying())
	{
		const float PitchLow = FMath::Lerp(EngineLowPitchMin, EngineLowPitchMax, EngineThrustSmoothed);
		const float VolLow = FMath::Lerp(EngineLowVolMin, EngineLowVolMax, EngineThrustSmoothed);

		EngineAudioLow->SetPitchMultiplier(PitchLow);
		EngineAudioLow->SetVolumeMultiplier(VolLow);
	}

	// High Layer
	const float HighT = FMath::Pow(EngineThrustSmoothed, 1.5f);

	if (EngineAudioHigh && EngineAudioHigh->IsPlaying())
	{
		const float PitchHigh = FMath::Lerp(EngineHighPitchMin, EngineHighPitchMax, HighT);
		const float VolHigh = FMath::Lerp(EngineHighVolMin, EngineHighVolMax, HighT);

		EngineAudioHigh->SetPitchMultiplier(PitchHigh);
		EngineAudioHigh->SetVolumeMultiplier(VolHigh);
	}
}

void APlayerChara::UpdateRotateAudio(float DeltaTime)
{
	if (!RotateAudio || !RotateAudio->IsPlaying())
	{
		return;
	}

	RotateInputSmoothed = FMath::FInterpTo(
		RotateInputSmoothed,
		m_RotateInput01,
		DeltaTime,
		RotateInterpSpeed
	);

	const float Pitch = FMath::Lerp(RotatePitchMin, RotatePitchMax, RotateInputSmoothed);
	const float Vol = FMath::Lerp(RotateVolMin, RotateVolMax, RotateInputSmoothed);

	RotateAudio->SetPitchMultiplier(Pitch);
	RotateAudio->SetVolumeMultiplier(Vol);
}

// ==================================================
// Course UI Update（主担当）
// ==================================================

void APlayerChara::UpdateCourseProgress(float DeltaTime)
{
	if (!m_pSpline || m_SplineLen <= 0.0f)
	{
		return;
	}

	const FVector Loc = GetActorLocation();

	// 最近点のキー→距離
	const float Key = m_pSpline->FindInputKeyClosestToWorldLocation(Loc);
	float S = m_pSpline->GetDistanceAlongSplineAtSplineInputKey(Key);
	S = FMath::Clamp(S, 0.0f, m_SplineLen);

	// 3Dコースで別区間に吸われるジャンプを抑制
	if (FMath::Abs(S - m_CourseSPrev) > m_CourseJumpLimit)
	{
		S = FMath::FInterpTo(m_CourseSPrev, S, DeltaTime, m_CourseUIInterp);
	}

	m_CourseSPrev = S;
	m_CourseS = S;

	// 逆走判定
	const FVector TangentDir =
		m_pSpline->GetDirectionAtDistanceAlongSpline(m_CourseS, ESplineCoordinateSpace::World);

	const FVector Vel = (m_pMovement ? m_pMovement->Velocity : FVector::ZeroVector);
	const float Dot = FVector::DotProduct(Vel.GetSafeNormal(), TangentDir);
	m_bReverse = (Dot < -0.2f);

	// UI表示は最大到達距離を使う
	m_CourseSBest = FMath::Max(m_CourseSBest, m_CourseS);

	// 表示用平滑化
	m_CourseSDisplay = FMath::FInterpTo(
		m_CourseSDisplay,
		m_CourseSBest,
		DeltaTime,
		m_CourseUIInterp
	);
	m_CourseSDisplay = FMath::Clamp(m_CourseSDisplay, 0.0f, m_SplineLen);
}

float APlayerChara::GetCourseProgress01() const
{
	if (m_SplineLen <= 0.0f)
	{
		return 0.0f;
	}

	return m_CourseSDisplay / m_SplineLen;
}

float APlayerChara::GetCourseRemainingDistance() const
{
	if (m_SplineLen <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, m_SplineLen - m_CourseSDisplay);
}

bool APlayerChara::IsReverseOnCourse() const
{
	return m_bReverse;
}