#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SquareHitBox.generated.h"

UCLASS()
class SWING_API ASquareHitBox : public AActor
{
	GENERATED_BODY()

public:
	//コンストラクタ
	ASquareHitBox();

protected:
	//トリガー判定用のBoxコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, CateGory = "GoalTrigger")
	UBoxComponent* TriggerBox;

	//トリガーにActorが侵入したときに呼ばれるイベント
	UFUNCTION(BlueprintNativeEvent, Category = "GoalTrigger")
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,	//重なった側のコンポーネント
		AActor* OtherActor,						//侵入してきたActor
		UPrimitiveComponent* OtherComp,			//侵入してきたActorのコンポーネント
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	//OnTriggerBeginOverlap のC++実装
	virtual void OnTriggerBeginOverlap_Implementation(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	//トリガーからActorが出たときに呼ばれるイベント
	UFUNCTION(BlueprintNativeEvent, Category = "GoalTrigger")
	void OnTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComp, //重なっていた側のコンポーネント
		AActor* OtherActor,                  //出ていったActor
		UPrimitiveComponent* OtherComp,      //出ていったActorのコンポーネント
		int32 OtherBodyIndex
	);

	//OnTriggerEndOverlap のC++実装
	virtual void OnTriggerEndOverlap_Implementation(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	//ゴール到達時の処理用イベント
	UFUNCTION(BlueprintImplementableEvent, Category = "GoalTrigger")
	void OnGoalTriggered(
		AActor* OverlappingActor,
		const TArray<FName>& ActorTags
	);
};
