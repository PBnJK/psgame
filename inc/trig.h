#ifndef GUARD_PSGAME_TRIG_H_
#define GUARD_PSGAME_TRIG_H_

#define ISIN_SHIFT (10)
#define ISIN2_SHIFT (15)
#define ISIN_PI (1 << (ISIN_SHIFT + 1))
#define ISIN2_PI (1 << (ISIN2_SHIFT + 1))

/* Calculates sin(x) */
int trig_isin(int x);

/* Calculates sin2(x) */
int trig_isin2(int x);

/* Calculates cos(x) */
static inline int trig_icos(int x) {
	return trig_isin(x + (1 << ISIN_SHIFT));
}

/* Calculates cos2(x) */
static inline int trig_icos2(int x) {
	return trig_isin2(x + (1 << ISIN2_SHIFT));
}

#endif // !GUARD_PSGAME_TRIG_H_
