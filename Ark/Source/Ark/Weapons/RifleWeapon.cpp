#include "RifleWeapon.h"

ARifleWeapon::ARifleWeapon()
{
	WeaponSlot = EFPSWeaponSlot::Primary;
	Damage = 30.f;
	Range = 15000.f;
	RefireRate = 0.10f;
	bFullAuto = true;
	FireMode = EFPSFireMode::Projectile;
	ProjectileInitialSpeed = 14000.f;
	MagazineSize = 30;
	MaxCarryAmmo = 180;
}
