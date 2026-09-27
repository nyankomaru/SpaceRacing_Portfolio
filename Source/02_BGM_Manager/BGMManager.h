#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BGMManager.generated.h"

// 前方宣言
class UAudioComponent;
class USoundBase;

/**
 * @brief BGMの再生・停止・切り替えを管理するActor
 *
 * BGM制御を専用Actorに集約し、Blueprint側から必要なタイミングで
 * 再生・停止・切り替えを呼び出せるようにしている。
 *
 * レベル内に配置して使用し、必要に応じてデフォルトBGMを自動再生できる。
 * 毎フレーム更新は不要なため、実装側ではTickを無効化している。
 */
UCLASS()
class SWING_API ABGMManager : public AActor
{
	GENERATED_BODY()

public:
	ABGMManager();

protected:
	/** ゲーム開始時に呼ばれる */
	virtual void BeginPlay() override;

public:
	// =========================
	// BGM操作
	// =========================

	/**
	 * BGMを再生する
	 * @param BGM 再生する音源
	 * @param FadeInTime フェードイン時間
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void PlayBGM(USoundBase* BGM, float FadeInTime = 0.5f);

	/**
	 * BGMを停止する
	 * @param FadeOutTime フェードアウト時間
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void StopBGM(float FadeOutTime = 0.5f);

	/**
	 * BGMを切り替える
	 * @param NewBGM 切り替え先の音源
	 * @param FadeOutTime フェードアウト時間
	 * @param FadeInTime フェードイン時間
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void ChangeBGM(USoundBase* NewBGM, float FadeOutTime = 0.5f, float FadeInTime = 0.5f);

private:
	// =========================
	// コンポーネント
	// =========================

	/** BGM再生に使うAudioComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* BGMComp = nullptr;

	// =========================
	// 設定
	// =========================

	/** 起動時に自動再生するBGM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true"))
	USoundBase* DefaultBGM = nullptr;

	/** BeginPlayでDefaultBGMを自動再生するか */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true"))
	bool bAutoPlayDefaultBGM = true;
};