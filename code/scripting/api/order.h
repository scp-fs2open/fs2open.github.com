#pragma once

#include <object/object.h>
#include <ai/ai.h>
#include <ship/ship.h>
#include "scripting/ade_api.h"

namespace scripting {
namespace api {


//**********HANDLE: order
struct order_h
{
	object_h objh;
	int odx;
	int sig;
	ai_goal *aigp;

	order_h();

	order_h(object *objp, int n_odx);

	bool IsValid();
};

DECLARE_ADE_OBJ(l_Order, order_h);

DECLARE_ADE_OBJ(l_ShipOrders, object_h);

}
}

