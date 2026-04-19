#include "SniperWeapon.h"

ASniperWeapon::ASniperWeapon()
{
	WeaponSlot = EFPSWeaponSlot::Primary;
	Damage = 80.f;
	Range = 30000.f;
	RefireRate = 1.0f;
	bFullAuto = false;
	MagazineSize = 5;
	MaxCarryAmmo = 30;
	FireMode = EFPSFireMode::Projectile;
	ProjectileInitialSpeed = 20000.f;
}
