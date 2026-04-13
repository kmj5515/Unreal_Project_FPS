#include "PistolWeapon.h"

APistolWeapon::APistolWeapon()
{
	WeaponSlot = EFPSWeaponSlot::Secondary;
	Damage = 20.f;
	Range = 10000.f;
	RefireRate = 0.22f;
	bFullAuto = false;
	FireMode = EFPSFireMode::Projectile;
	ProjectileInitialSpeed = 11000.f;
}
