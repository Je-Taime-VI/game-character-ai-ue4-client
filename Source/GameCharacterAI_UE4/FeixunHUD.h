#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FeixunHUD.generated.h"

class SFeixunChatWidget;
class USoundBase;

UCLASS()
class GAMECHARACTERAI_UE4_API AFeixunHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void ToggleFeixunPanel();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feixun|Audio")
    USoundBase* ClickSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feixun|Audio")
    USoundBase* SendSound;

private:
    TSharedPtr<SFeixunChatWidget> ChatWidget;
};
