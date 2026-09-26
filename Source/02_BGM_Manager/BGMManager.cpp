#include "BGMManager.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

// =========================
// コンストラクタ
// =========================

ABGMManager::ABGMManager()
{
	// BGM管理では毎フレーム更新が不要なため、Tickは無効化
	PrimaryActorTick.bCanEverTick = false;

	// BGM再生用のAudioComponentを作成
	BGMComp = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComp"));
	RootComponent = BGMComp;

	// 自動再生は行わず、必要なタイミングで再生する
	BGMComp->bAutoActivate = false;

	// BGMとして扱いやすいようにUIサウンド設定にする
	BGMComp->bIsUISound = true;

	// ループ設定は音源側で行う想定
}

// =========================
// 開始時処理
// =========================

void ABGMManager::BeginPlay()
{
	Super::BeginPlay();

	// 必要なら開始時にデフォルトBGMを再生
	if (bAutoPlayDefaultBGM && DefaultBGM)
	{
		PlayBGM(DefaultBGM, 0.5f);
	}
}

// =========================
// BGM再生
// =========================

void ABGMManager::PlayBGM(USoundBase* BGM, float FadeInTime)
{
	// 音源やコンポーネントが無い場合は何もしない
	if (!BGMComp || !BGM)
	{
		return;
	}

	// 再生する音源を設定
	BGMComp->SetSound(BGM);

	// フェードイン指定があればフェード付きで再生
	if (FadeInTime > 0.0f)
	{
		BGMComp->FadeIn(FadeInTime, 1.0f);
	}
	else
	{
		BGMComp->Play();
	}
}

// =========================
// BGM停止
// =========================

void ABGMManager::StopBGM(float FadeOutTime)
{
	// コンポーネントが無い場合は何もしない
	if (!BGMComp)
	{
		return;
	}

	// フェードアウト指定があれば徐々に停止
	if (FadeOutTime > 0.0f)
	{
		BGMComp->FadeOut(FadeOutTime, 0.0f);
	}
	else
	{
		BGMComp->Stop();
	}
}

// =========================
// BGM切り替え
// =========================

void ABGMManager::ChangeBGM(USoundBase* NewBGM, float FadeOutTime, float FadeInTime)
{
	// 音源やコンポーネントが無い場合は何もしない
	if (!BGMComp || !NewBGM)
	{
		return;
	}

	// 再生中なら一度フェードアウトをかけてから差し替える
	if (BGMComp->IsPlaying() && FadeOutTime > 0.0f)
	{
		BGMComp->FadeOut(FadeOutTime, 0.0f);
		BGMComp->SetSound(NewBGM);

		if (FadeInTime > 0.0f)
		{
			BGMComp->FadeIn(FadeInTime, 1.0f);
		}
		else
		{
			BGMComp->Play();
		}
	}
	else
	{
		// 停止中ならそのまま再生開始
		BGMComp->Stop();
		PlayBGM(NewBGM, FadeInTime);
	}
}