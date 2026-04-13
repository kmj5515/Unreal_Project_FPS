#include "SniperWeapon.h"

ASniperWeapon::ASniperWeapon()
{
	WeaponSlot = EFPSWeaponSlot::Primary;
	Damage = 80.f;
	Range = 30000.f;
	RefireRate = 1.0f;
	bFullAuto = false;
	MagazineSize = 5;
	FireMode = EFPSFireMode::Projectile;
	ProjectileInitialSpeed = 20000.f;
}
