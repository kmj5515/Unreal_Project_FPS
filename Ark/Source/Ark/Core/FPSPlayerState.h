#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

class UAbilitySystemComponent;
class UFPSAttributeSet;

UCLASS()
class ARK_API AFPSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFPSPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFPSAttributeSet* GetAttributeSet() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UFPSAttributeSet> AttributeSet;
};
