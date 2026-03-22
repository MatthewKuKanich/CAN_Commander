#pragma once

#include "can_commander.h"

void controller_start();
void controller_stop();
void controller_enable();
void controller_handle(const CcEvent* event);

bool controller_is_enabled();