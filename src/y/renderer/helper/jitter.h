/*
 * jitter.h
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */

#pragma once

struct mat4;

mat4 jitter(float w, float h, int uid);
void jitter_iterate();

