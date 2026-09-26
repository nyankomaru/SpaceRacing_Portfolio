#pragma once

#include "CoreMinimal.h"
#include "MyPawn.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "PlayerChara.generated.h"

// 前方宣言
class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UFloatingPawnMovement;
class USplineComponent;
class APlanet;
class AMyCamera;

/**
 * プレイヤーキャラクタークラス
 *
 * 主な役割：
 * ・プレイヤーの移動、回転、速度制御
 * ・カメラ追従や視野角制御
 * ・コース（Spline）に沿った進行制御
 * ・進捗UI用の進行度計算
 * ・エンジン音、回転音などのAudio制御
 *
 * 補足：
 * 本クラスは複数機能をまとめて持つプレイヤー基盤クラスであり、
 * UI進行度表示とAudio周りは主担当として調整・整理を行っている。
 * それ以外の既存処理についても、全体の見通しをよくする目的で構造を整理している。
 */
UCLASS()
class SWING_API APlayerChara : public AMyPawn
{
	GENERATED_BODY()

public:
	APlayerChara();

public:
	// =========================
	// Engine Override
	// =========================

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// =========================
	// Overlap
	// =========================

	/** オーバーラップ開始時 */
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** オーバーラップ終了時 */
	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

public:
	// =========================
	// Public Utility
	// =========================

	UFUNCTION(BlueprintCallable)
	void AddSocket(AActor* _p, FVector _Pos, bool _bRot = false);

	UFUNCTION(BlueprintCallable)
	void RemoveSocket(AActor* _p);

	UFUNCTION(BlueprintCallable)
	void ChangeCamera(USpringArmComponent* _pSpring, UCameraComponent* _pCamera);

	UFUNCTION(BlueprintCallable)
	int GetGraNum();

	UFUNCTION(BlueprintCallable)
	float GetSpeed();

	UFUNCTION(BlueprintCallable)
	float GetMaxSpeed();

	UFUNCTION(BlueprintPure, Category = "Input")
	float GetForwardInput() const;

	UFUNCTION(BlueprintCallable)
	float GetForwardInputTime() const;

	FRotator GetUpRotator();

	UPrimitiveComponent* GetSpline() const;

	/** 開始位置・回転・速度を強制設定する */
	UFUNCTION(BlueprintCallable)
	void SetStart(FVector _Loc, FRotator _Rot, float _Speed);

	/** 現在速度を減算する */
	void SubSpeed(float _Rate);

public:
	// =========================
	// Course UI
	// 主担当：進行度表示用
	// =========================

	/** コース進行度を 0.0 ～ 1.0 で返す */
	UFUNCTION(BlueprintPure, Category = "Course|UI")
	float GetCourseProgress01() const;

	/** ゴールまでの残り距離を返す */
	UFUNCTION(BlueprintPure, Category = "Course|UI")
	float GetCourseRemainingDistance() const;

	/** 逆走中かどうかを返す */
	UFUNCTION(BlueprintPure, Category = "Course|UI")
	bool IsReverseOnCourse() const;

private:
	// =========================
	// Update
	// =========================

	void UpdateRotation(float DeltaTime);
	void UpdateMove(float DeltaTime);

	void UpdateCamera(float DeltaTime);
	void UpdateCameraMove(float DeltaTime);
	void UpdateCameraRot(float DeltaTime);
	void UpdateCameraFOV(float DeltaTime);

	void UpdateSocket();
	void UpdateCourseProgress(float DeltaTime);
	void ValueReset();

private:
	// =========================
	// Input
	// =========================

	void MoveForward(float _value);
	void Deceleration(float _value);

	void RotPitch(float _value);
	void RotYaw(float _value);
	void RotRoll(float _value);

	void CameraRotPitch(float _value);
	void CameraRotYaw(float _value);

	void ChangeCtrl(float _value);
	void ChangeCollision();
	void ChangeCamCon();
	void ChangeAutoRot();

private:
	// =========================
	// Audio Update
	// 主担当：BGM / Audio 関連
	// =========================

	void UpdateEngineAudio(float DeltaTime);
	void UpdateRotateAudio(float DeltaTime);

private:
	// =========================
	// Collision / Mesh
	// =========================

	/** メインコリジョン */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* m_pMainCollision = nullptr;

	/** プレイヤーメッシュ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* m_pMesh = nullptr;

private:
	// =========================
	// Camera
	// =========================

	/** スプリングアーム */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* m_pSpring = nullptr;

	/** 通常時のスプリングアーム位置 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	FVector m_DefaAddSpringPos;

	/** メインカメラ */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* m_pCamera = nullptr;

	/** 通常時のカメラ角度 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	FRotator m_DefaCameraRot;

	/** カメラ回転速度 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_CameraRotSpeed = 0.0f;

	/** カメラ復帰速度 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_CameraReturnRotSpeed = 0.0f;

	/** カメラ距離変化速度 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_CameraLagDistanceSpeed = 0.0f;

	/** カメラ最大距離 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_CameraLagMaxDistance = 0.0f;

	/** 通常FOV */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_DefaFOV = 0.0f;

	/** 追加FOV最大値 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_MaxAddFOV = 0.0f;

	/** 加速時追加FOV */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_SpeedUpFOV = 0.0f;

	/** 通常時FOV変化速度 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_DefaAddFOVSpeed = 0.0f;

	/** 加速時FOV変化速度 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_SpeedUpAddFOVSpeed = 0.0f;

private:
	// =========================
	// Move
	// =========================

	/** 移動コンポーネント */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (AllowPrivateAccess = "true"))
	UFloatingPawnMovement* m_pMovement = nullptr;

	/** 前進速度 */
	UPROPERTY(EditAnywhere, Category = "Move", meta = (AllowPrivateAccess = "true"))
	float m_ForwardSpeed = 0.0f;

	/** コース復帰速度 */
	UPROPERTY(EditAnywhere, Category = "Move", meta = (AllowPrivateAccess = "true"))
	float m_ReturnCourseSpeed = 0.0f;

	/** コース復帰最低速度 */
	UPROPERTY(EditAnywhere, Category = "Move", meta = (AllowPrivateAccess = "true"))
	float m_MinReturnCourseSpeed = 0.0f;

	/** コースアウト時減速率 */
	UPROPERTY(EditAnywhere, Category = "Move", meta = (AllowPrivateAccess = "true"))
	float M_CourseOutRate = 0.0f;

	/** 前進入力値 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float m_ForwardInput = 0.0f;

private:
	// =========================
	// Rotation
	// =========================

	/** 最大回転速度 */
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float m_MaxRotSpeed = 0.0f;

	/** 最大回転速度に達する基準速度 */
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float m_ReachMaxRotSpeed = 0.0f;

	/** 進行方向へ戻る回転速度 */
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float m_ReturnRotSpeed = 0.0f;

	/** 回転中心 */
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	FVector m_RotPivot;

private:
	// =========================
	// Course / Spline
	// =========================

	/** コーススプライン */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course", meta = (AllowPrivateAccess = "true"))
	USplineComponent* m_pSpline = nullptr;

	/** コース復帰距離 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course", meta = (AllowPrivateAccess = "true"))
	float m_ReturnCourseLen = 0.0f;

private:
	// =========================
	// Course UI Parameter
	// 主担当：進行度表示用
	// =========================

	/** コース全長 */
	float m_SplineLen = 0.0f;

	/** 現在のスプライン距離 */
	float m_CourseS = 0.0f;

	/** 前フレームのスプライン距離 */
	float m_CourseSPrev = 0.0f;

	/** 最大到達距離（ゲージを戻さない用） */
	float m_CourseSBest = 0.0f;

	/** UI表示用の平滑化距離 */
	float m_CourseSDisplay = 0.0f;

	/** 逆走中かどうか */
	bool m_bReverse = false;

	/** 距離ジャンプ判定しきい値 */
	UPROPERTY(EditAnywhere, Category = "Course|UI", meta = (AllowPrivateAccess = "true"))
	float m_CourseJumpLimit = 8000.0f;

	/** UI補間速度 */
	UPROPERTY(EditAnywhere, Category = "Course|UI", meta = (AllowPrivateAccess = "true"))
	float m_CourseUIInterp = 8.0f;

private:
	// =========================
	// Audio Component
	// 主担当：BGM / Audio 関連
	// =========================

	/** 低域エンジン音 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* EngineAudioLow = nullptr;

	/** 高域エンジン音 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* EngineAudioHigh = nullptr;

	/** 回転音 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* RotateAudio = nullptr;

	/** 低域エンジン音源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	USoundBase* EngineLowLoopSound = nullptr;

	/** 高域エンジン音源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	USoundBase* EngineHighLoopSound = nullptr;

	/** 回転音源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	USoundBase* RotateLoopSound = nullptr;

private:
	// =========================
	// Audio Parameter
	// 主担当：BGM / Audio 関連
	// =========================

	/** 推力入力値（0～1） */
	float m_ThrustInput01ThisFrame = 0.0f;

	/** 回転入力値（0～1） */
	float m_RotateInput01 = 0.0f;

	// ---- Engine Low ----
	UPROPERTY(EditAnywhere, Category = "Audio|EngineLow", meta = (AllowPrivateAccess = "true"))
	float EngineLowPitchMin = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Audio|EngineLow", meta = (AllowPrivateAccess = "true"))
	float EngineLowPitchMax = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Audio|EngineLow", meta = (AllowPrivateAccess = "true"))
	float EngineLowVolMin = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Audio|EngineLow", meta = (AllowPrivateAccess = "true"))
	float EngineLowVolMax = 0.6f;

	// ---- Engine High ----
	UPROPERTY(EditAnywhere, Category = "Audio|EngineHigh", meta = (AllowPrivateAccess = "true"))
	float EngineHighPitchMin = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Audio|EngineHigh", meta = (AllowPrivateAccess = "true"))
	float EngineHighPitchMax = 1.6f;

	UPROPERTY(EditAnywhere, Category = "Audio|EngineHigh", meta = (AllowPrivateAccess = "true"))
	float EngineHighVolMin = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Audio|EngineHigh", meta = (AllowPrivateAccess = "true"))
	float EngineHighVolMax = 0.8f;

	/** エンジン音補間速度 */
	UPROPERTY(EditAnywhere, Category = "Audio|Engine", meta = (AllowPrivateAccess = "true"))
	float EngineInterpSpeed = 8.0f;

	/** エンジン入力平滑化値 */
	float EngineThrustSmoothed = 0.0f;

	// ---- Rotate ----
	UPROPERTY(EditAnywhere, Category = "Audio|Rotate", meta = (AllowPrivateAccess = "true"))
	float RotatePitchMin = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Audio|Rotate", meta = (AllowPrivateAccess = "true"))
	float RotatePitchMax = 1.3f;

	UPROPERTY(EditAnywhere, Category = "Audio|Rotate", meta = (AllowPrivateAccess = "true"))
	float RotateVolMin = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Audio|Rotate", meta = (AllowPrivateAccess = "true"))
	float RotateVolMax = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Audio|Rotate", meta = (AllowPrivateAccess = "true"))
	float RotateInterpSpeed = 10.0f;

	/** 回転入力平滑化値 */
	float RotateInputSmoothed = 0.0f;

private:
	// =========================
	// Existing State
	// 担当外だが見通し改善のため整理
	// =========================

	TArray<APlanet*> m_pPlanets;

	TArray<AActor*> m_pSocket;
	TArray<FVector> m_SocketPos;
	TArray<bool> m_bSocketRot;

	FVector m_PreLoc;
	FVector m_Velocity;
	FVector m_ReturnCourseVelo;

	FRotator m_Rot;
	FRotator m_PreRotIn;
	FRotator m_NowRotSpeed;
	FRotator m_CameraRot;
	FRotator m_CameraRotInput;

	float m_PreForwardInput = 0.0f;
	float m_Speed = 0.0f;
	float m_ForwardInputTime = 0.0f;
	float m_StrongFOVTimer = 0.0f;

	float m_ChangeCtrl = 0.0f;

	bool m_bCollisiON = false;
	bool m_bCamConChange = false;
	bool m_bAutoRot = false;
	bool m_bReturnCource = false;
};