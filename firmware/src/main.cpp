#include "Fixtures.h"
#include "Core/Engine.h"
// #include "CoreV2/Grid.h"
// #include "OutputV2/HardwareLEDStrip.h"

void setup() 
{	
	// OutputV2::HardwareLEDStrip strip_1(OutputV2::LEDManifest<WS2815, 5, RGB>{}, 8 * 24);
	// OutputV2::HardwareLEDStrip strip_2(OutputV2::LEDManifest<WS2815, 6, RGB>{}, 8 * 24);

	// CoreV2::LEDStripMapping mapping_1{
	// 	&strip_1,
	// 	CoreV2::Range{0, 4},
	// 	2,
	// 	CoreV2::Range{0, 12},
	// 	2
	// };

	// CoreV2::LEDStripMapping mapping_2{
	// 	&strip_2,
	// 	CoreV2::Range{4, 4},
	// 	2,
	// 	CoreV2::Range{0, 12},
	// 	2
	// };

	// CoreV2::Grid grid(CoreV2::Grid::Config{8, 12});

	// grid.addMapping(mapping_1);
	// grid.addMapping(mapping_2);
	// // After this point all entries of 8*12 grid are mapped to appropriate update function.


	// uint8_t* data_1;
	// CoreV2::UniverseUpdate update_1{1, data_1, CoreV2::Range{0, 4}, CoreV2::Range{0, 12}};
	// uint8_t* data_2;
	// CoreV2::UniverseUpdate update_2{2, data_2, CoreV2::Range{4, 8}, CoreV2::Range{0, 12}};

	// grid.update(update_1);
	// grid.update(update_2);

	Engine::instance().addFixture<Astera60>(true);
	Engine::instance().init();
}

void loop()
{
	vTaskDelete(NULL);
}













