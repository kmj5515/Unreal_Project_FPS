#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

class UAbilitySystemComponent;
class UFPSAttributeSet;
class UGameplayEffect;

UCLASS()
class ARK_API AFPSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFPSPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFPSAttributeSet* GetAttributeSet() const;

	void TryApplyDefaultAttributes();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UFPSAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	bool bDefaultAttributesApplied = false;
};
