#include "SquareHitBox.h"
#include "Engine/Engine.h"

ASquareHitBox::ASquareHitBox()
{
    //Tickは不要なので無効化（軽量化）
    PrimaryActorTick.bCanEverTick = false;

    //トリガー用Boxコンポーネント生成
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));

    //Boxサイズ設定（必要に応じてエディタやコードで調整）
    TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 100.f));

    //クエリ専用（Overlap判定のみ使用）
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    //全てのチャンネルに対してオーバーラップを許可
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);

    //オーバーラップイベントを有効化
    TriggerBox->SetGenerateOverlapEvents(true);

    //ゲーム中は非表示（デバッグ用に可視化したい場合はfalse）
    TriggerBox->SetHiddenInGame(true);

    //RootComponentとして設定
    RootComponent = TriggerBox;

    /** オーバーラップ開始・終了イベントをバインド
    * BlueprintNativeEventでもC++実装関数を直接指定できる*/
    TriggerBox->OnComponentBeginOverlap.AddDynamic(
        this, &ASquareHitBox::OnTriggerBeginOverlap
    );
    TriggerBox->OnComponentEndOverlap.AddDynamic(
        this, &ASquareHitBox::OnTriggerEndOverlap
    );
}

void ASquareHitBox::OnTriggerBeginOverlap_Implementation(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    // 自分自身でなければ処理
    if (OtherActor && OtherActor != this)
    {
        // 侵入してきたActorのタグ一覧を取得
        const TArray<FName>& ActorTags = OtherActor->Tags;

#if !UE_BUILD_SHIPPING
        // デバッグ用：Actor名とタグをログ出力
        FString TagString;
        for (const FName& Tag : ActorTags)
        {
            TagString += Tag.ToString() + TEXT(" ");
        }

        UE_LOG(
            LogTemp,
            Log,
            TEXT("GoalTriggerVolume overlapped by %s with tags: %s"),
            *OtherActor->GetName(),
            *TagString
        );
#endif

        /*ゴール到達イベントを呼び出し
        ・実際のゴール処理はBlueprint側で実装する想定*/
        OnGoalTriggered(OtherActor, ActorTags);
    }
}

void ASquareHitBox::OnTriggerEndOverlap_Implementation(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    /** トリガーからActorが離れたときの処理
    * ・現時点では未使用
    * ・ゴール解除、再突入判定などに利用可能*/
}

