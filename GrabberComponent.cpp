
#include "GrabberComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UGrabberComponent::UGrabberComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UGrabberComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UGrabberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();
	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent() != nullptr)
	{
		//set target location/rotation of grabbed actor
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}

void UGrabberComponent::Grab()
{
	//check for physics handle 
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle(); 
	if (PhysicsHandle == nullptr)
	{
		return;
	}

	FHitResult HitResult;
	bool HasHit = GetGrabbableInReach(HitResult);
		
	if (HasHit)
	{
		//wake rigid bodies
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		HitComponent->SetSimulatePhysics(true);
		HitComponent->WakeAllRigidBodies();
		AActor* HitActor = HitResult.GetActor();

		//add 'grabbed' tag
		HitActor->Tags.Add("Grabbed");
		HitActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		//grab grabbable actor
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			HitResult.GetComponent(),
			NAME_None,
			HitResult.ImpactPoint,
			GetComponentRotation()
		);
	}
}

void UGrabberComponent::Release()
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent()) 
	{
		PhysicsHandle->GetGrabbedComponent()->WakeAllRigidBodies();

		//remove 'grabbed' tag
		AActor* GrabbedActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();
		GrabbedActor->Tags.Remove("Grabbed");
		
		PhysicsHandle->ReleaseComponent();
	}
}


UPhysicsHandleComponent* UGrabberComponent::GetPhysicsHandle() const
{
	UPhysicsHandleComponent* Result = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (Result == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Grabber requies a physics handle component"));
	}
	return Result;
}

bool UGrabberComponent::GetGrabbableInReach(FHitResult& OutHitResult) const
{
	//collision sphere check for grabbable actors & return hit result
	//Line trace set up
	FVector Start = GetComponentLocation(); 
	FVector End = Start + GetForwardVector() * MaxGrabDistance;  

	//collision sphere set up
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius); 
	return GetWorld()->SweepSingleByChannel( 
		OutHitResult, 
		Start, End, 
		FQuat::Identity, 
		ECC_GameTraceChannel2, 
		Sphere);
}
