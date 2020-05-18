// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/StrategySelectionComponent.h"

#include "DrawDebugHelpers.h"
#include "SelectableActor.h"
#include "StrategyHUD.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Project_Timewars/Public/StrategyHelpers.h"

UStrategySelectionComponent::UStrategySelectionComponent()
{
    isSelecting = false;
}

void UStrategySelectionComponent::BeginPlay()
{
    PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!ensure(PlayerController != nullptr)) return;

    HUD = Cast<AStrategyHUD>(PlayerController->GetHUD());
}

void UStrategySelectionComponent::TickComponent(float DeltaTime,enum ELevelTick TickType,FActorComponentTickFunction * ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UStrategySelectionComponent::StartSelection()
{
    if (!ensure(HUD != nullptr)) return;
    if (!StrategyHelpers::GetMousePosition(SelectionStartPoint, PlayerController)) return;
    
    HUD->StartSelection();    
    isSelecting = true;
}

void UStrategySelectionComponent::EndSelection()
{
    TArray<ASelectableActor*> Selection;

    auto World = GetWorld();
    if (!ensure(World != nullptr)) return;
    if (!ensure(HUD != nullptr)) return;

    // Get selection endpoint
    if (!StrategyHelpers::GetMousePosition(SelectionEndPoint, PlayerController))
    {
        return;
    }
    
    // If end and start points are near enough, select only as 1 point
    if (FVector2D::Distance(SelectionEndPoint, SelectionStartPoint) <= MaxSelectionApprox)
    {
        Selection = MakeSingleSelection();
    } else
    {
        // Else select from area
        Selection = MakeMultipleSelection();
    }

    HUD->EndSelection(); 
    ResetSelection();
    SelectActors(Selection);

    //UE_LOG(LogTemp, Warning, TEXT("Selected actors: %d"), Selection.Num())
}

TArray<ASelectableActor*> UStrategySelectionComponent::MakeSingleSelection()
{
    TArray<AActor*> outActors;
    TArray<ASelectableActor*> selectableActors;

    FVector WorldPosition;
    if (!StrategyHelpers::DeprojectPositionToWorld(SelectionEndPoint, WorldPosition, PlayerController, ECollisionChannel::ECC_WorldDynamic)) return selectableActors;
    
    // Check overlapping actors to selection sphere
    TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
    traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
    
    UClass* selectableClass = ASelectableActor::StaticClass();
    const TArray<AActor*> ignoreActors;
    
    // DrawDebugSphere(GetWorld(), WorldPosition, MaxSelectionApprox, 12, FColor::Red, true, 15.0f);
    UKismetSystemLibrary::SphereOverlapActors(this, WorldPosition, MaxSelectionApprox, traceObjectTypes, selectableClass, ignoreActors, outActors);
    
    for (auto a : outActors)
    {
        selectableActors.Add(Cast<ASelectableActor>(a));
    }
    
    return selectableActors;
}

TArray<ASelectableActor*> UStrategySelectionComponent::MakeMultipleSelection()
{
    TArray<ASelectableActor*> outActors;
    
    if (!ensure(PlayerController != nullptr)) return outActors;
    if (!ensure(HUD != nullptr)) return outActors;
   
    // Select all actors inside frustum
    outActors = HUD->GetCurrentSelection();
    return outActors;
}

void UStrategySelectionComponent::ResetSelection()
{
    for (auto Actor : SelectedActors)
    {
        Actor->SetActorSelected(false);
    }
    SelectedActors.Empty();
}

void UStrategySelectionComponent::SelectActors(TArray<ASelectableActor*> selectedActors)
{
    for (auto Actor : selectedActors)
    {
        Actor->SetActorSelected(true);    
    }
    SelectedActors = selectedActors;
}                                
