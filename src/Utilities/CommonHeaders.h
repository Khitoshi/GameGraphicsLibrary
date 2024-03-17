#pragma once

// C/C++
#include <stdint.h>
#include <assert.h>
#include <typeinfo>
#include <memory>
#include <unordered_map>

#if defined(_WIN64)
#include <DirectXMath.h>
#endif

#include "..\Utilities\MathTypes.h"
#include "..\Utilities\PrimitiveTypes.h"
#include "..\Utilities\ContainerTypes.h"
#include "..\Utilities\Id.h"

#ifdef _DEBUG
#define DEBUG_OP(x) x
#else
#define DEBUG_OP(x) (void(0))
#endif