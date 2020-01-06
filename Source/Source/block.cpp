#include "stdafx.h"
#include "pipeline.h"
#include "block.h"

const std::vector<BlockProperties> Block::PropertiesTable =
{
	{BlockProperties("air",        0,		{0, 0, 0, 0},							{0, 0, 0, 0})},
	{BlockProperties("stone",      32,	{.4f, .4f, .4f, 1},				{0, 0, 0, 0})},
	{BlockProperties("dirt",       16,	{.6f, .3f, .1f, 1},				{0, 0, 0, 0})},
	{BlockProperties("metal",      128,	{.9f, .9f, 1.f, 1},				{0, 0, 0, 0})},
	{BlockProperties("grass",      8,		{.3, 1, 0, 1},						{0, 0, 0, 0})},
	{BlockProperties("sand",       4,		{.761f, .698f, .502f, 1},	{0, 0, 0, 0})},
	{BlockProperties("snow",       4,		{1, .98f, .98f, 1},				{0, 0, 0, 0})},
	{BlockProperties("water",      256,	{0, .476f, .745f, .9f},		{0, 0, 0, 0})},
	{BlockProperties("oak wood",   2,		{.729f, .643f, .441f, 1},	{0, 0, 0, 0})},
	{BlockProperties("oak leaves", 2,		{.322f, .42f, .18f, 1},		{0, 0, 0, 0})},
	{BlockProperties("error",      64,	{1, 0, 0, 1},							{0, 0, 0, 0})},
	{BlockProperties("dry grass",  8,		{.75, 1, 0, 1},						{0, 0, 0, 0})},
	{BlockProperties("Olight",		 8,		{1, .5, 0, 1},						{15, 8, 0, 0})},
	{BlockProperties("Rlight",		 8,		{1, 0, 0, 1},							{15, 0, 0, 0})},
	{BlockProperties("Glight",		 8,		{0, 1, 0, 1},							{0, 15, 0, 0})},
	{BlockProperties("Blight",		 8,		{0, 0, 1, 1},							{0, 0, 15, 0})},
	{BlockProperties("Smlight",		 8,		{.5, .5, .5, 1},					{15, 15, 15, 0})},
};
