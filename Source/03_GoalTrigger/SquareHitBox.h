#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SquareHitBox.generated.h"

/**
 * @brief ゴール判定用のBoxトリガーActor
 *
 * C++側ではOverlap判定とActorタグ取得を担当し、
 * ゴール到達時の演出やリザルト処理はBlueprint側で実装できるようにしている。
 *
 * BlueprintImplementableEventを通して処理を渡すことで、
 * 判定処理と演出処理を分離し、拡張しやすい構成にしている。
 */
UCLASS()
class SWING_API ASquareHitBox : public AActor
{
	GENERATED_BODY()

public:
	// コンストラクタ
	ASquareHitBox();

protected:
	// トリガー判定用のBoxコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GoalTrigger")
	UBoxComponent* TriggerBox;

	// トリガーにActorが侵入したときに呼ばれるイベント
	UFUNCTION(BlueprintNativeEvent, Category = "GoalTrigger")
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,	//重なった側のコンポーネント
		AActor* OtherActor,						//侵入してきたActor
		UPrimitiveComponent* OtherComp,			//侵入してきたActorのコンポーネント
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// OnTriggerBeginOverlap のC++実装
	virtual void OnTriggerBeginOverlap_Implementation(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// トリガーからActorが出たときに呼ばれるイベント
	UFUNCTION(BlueprintNativeEvent, Category = "GoalTrigger")
	void OnTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComp, //重なっていた側のコンポーネント
		AActor* OtherActor,                  //出ていったActor
		UPrimitiveComponent* OtherComp,      //出ていったActorのコンポーネント
		int32 OtherBodyIndex
	);

	// OnTriggerEndOverlap のC++実装
	virtual void OnTriggerEndOverlap_Implementation(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	// ゴール到達時の処理用イベント
	UFUNCTION(BlueprintImplementableEvent, Category = "GoalTrigger")
	void OnGoalTriggered(
		AActor* OverlappingActor,
		const TArray<FName>& ActorTags
	);
};
