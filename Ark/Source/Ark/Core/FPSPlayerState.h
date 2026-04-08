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

	/** 스폰 시 1회 적용(서버). 에디터에서 `GE_DefaultAttributes` 등 지정. */
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
