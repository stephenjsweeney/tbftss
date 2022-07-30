/*
Copyright (C) 2015-2019,2022 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

void destroyEffects(void);
void addECMEffect(Entity *ent);
void addShieldSplinterEffect(Entity *ent);
void addMissileEngineEffect(Bullet *b);
void addLargeEngineEffect(void);
void addEngineEffect(void);
void addMissileExplosion(Bullet *b);
void addLargeExplosion(void);
void addMineExplosion(void);
void addSmallExplosion(void);
void addDebrisFire(int x, int y);
void addSmallFighterExplosion(void);
void addBulletHitEffect(int x, int y, int r, int g, int b);
void drawShieldHitEffect(Entity *e);
void drawEffects(void);
void doEffects(void);
void initEffects(void);
