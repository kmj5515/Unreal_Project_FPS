#include "KnifeWeapon.h"

AKnifeWeapon::AKnifeWeapon()
{
	WeaponSlot = EFPSWeaponSlot::Melee;
	Damage = 50.f;
	bFullAuto = false;
	RefireRate = 0.3f;
	MeleeRange = 180.f;
	MeleeRadius = 30.f;
}
