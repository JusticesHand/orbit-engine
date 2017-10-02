/*! @file OrbitMainModule.h */

#ifndef ORBITMAINMODULE_H
#define ORBITMAINMODULE_H
#pragma once

#include "Game/MainModule.h"

namespace OrbitMain
{
	class OrbitMainModule final : public Orbit::MainModule
	{
	public:
		OrbitMainModule() = default;
		virtual ~OrbitMainModule() = default;

		void load() override;
		void unload() override;
		std::unique_ptr<Orbit::Scene> getInitialScene() override;
	};
}

#endif //ORBITMAINMODULE_H