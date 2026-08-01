#include "FeixunHUD.h"
#include "Components/InputComponent.h"
#include "SFeixunChatWidget.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Widgets/SWeakWidget.h"

void AFeixunHUD::BeginPlay()
{
    Super::BeginPlay();

    if (GEngine && GEngine->GameViewport)
    {
        SAssignNew(ChatWidget, SFeixunChatWidget)
            .OwningHUD(this)
            .ClickSound(ClickSound)
            .SendSound(SendSound);

        GEngine->GameViewport->AddViewportWidgetContent(
            SNew(SWeakWidget).PossiblyNullContent(ChatWidget.ToSharedRef()),
            10
        );

        if (APlayerController* PC = GetOwningPlayerController())
        {
            PC->bShowMouseCursor = true;
            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(ChatWidget);
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            FSlateApplication::Get().SetKeyboardFocus(ChatWidget);

            EnableInput(PC);
            if (InputComponent)
            {
                InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AFeixunHUD::ToggleFeixunPanel);
            }
        }
    }
}

void AFeixunHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GEngine && GEngine->GameViewport && ChatWidget.IsValid())
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(ChatWidget.ToSharedRef());
    }
    ChatWidget.Reset();

    Super::EndPlay(EndPlayReason);
}



void AFeixunHUD::ToggleFeixunPanel()
{
    if (ChatWidget.IsValid())
    {
        ChatWidget->TogglePanel();
    }
}
