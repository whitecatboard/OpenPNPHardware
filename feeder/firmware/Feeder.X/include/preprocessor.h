/*
 *          /\       /\
 *         /  \_____/  \
 *        /_____________\
 *        W H I T E C A T
 *
 * Copyright (C) 2020, IBEROXARXA SERVICIOS INTEGRALES, S.L.
 * Copyright (C) 2020, Jaume Oliv� Petrus (jolive@whitecatboard.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREPROCESSOR_H
#define	PREPROCESSOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PREP_PASTER(x,y) x##y
#define PREP_EVALUATOR(x,y) PREP_PASTER(x,y)
#define PREP_CONCAT(x,y) PREP_EVALUATOR(x,y)

#ifdef	__cplusplus
}
#endif

#endif	/* PREPROCESSOR_H */

