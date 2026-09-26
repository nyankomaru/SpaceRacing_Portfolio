#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BGMManager.generated.h"

// 前方宣言
class UAudioComponent;
class USoundBase;

/**
 * BGMを管理するActor
 *
 * 主な役割：
 * ・BGMの再生
 * ・BGMの停止
 * ・BGMの切り替え
 *
 * レベル内に配置して使用し、
 * 必要に応じてデフォルトBGMを自動再生する。
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