// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class DEMO_API FGameUtils
{
public:
	/**
	 * @brief 添加一个RetryTimer
	 * @tparam FunctorType 
	 * @param Callback 
	 * @param Context 
	 * @param Handle 
	 * @param Rate 
	 */
	template <typename FunctorType>
	static void SetRetryTimer(FunctorType&& Callback,UObject* Context, FTimerHandle& Handle, float Rate)
	{
		if (Callback()) 
		{
			return; 
		}

		if (Context && Context->GetWorld())
		{
			Context->GetWorld()->GetTimerManager().SetTimer(
				Handle,
				FTimerDelegate::CreateWeakLambda(Context, [Callback = MoveTemp(Callback), Context, &Handle]() mutable
				{
					if (Callback())
					{
						if (GEngine) 
						{
							if (UWorld* World = GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::LogAndReturnNull))
							{
								World->GetTimerManager().ClearTimer(Handle);
							}
						}
					}
				}),
				Rate,
				true 
			);
		}
	}
	FGameUtils();
	~FGameUtils();
};
