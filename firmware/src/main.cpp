// #include "Fixtures.h"
// #include "Core/Engine.h"
// // #include "Config/config.h"

// // #include "CoreV2/Grid.h"
// // #include "OutputV2/HardwareLEDStrip.h"
// // #include "OutputV2/LEDStrip.h"
// // #include "OutputV2/LEDGrid.h"
// // #include "OutputV2/VirtualGrid.h"
// void setup()
// {
//     // Config cfg{
//     //     {
//     //         GroupConfig(
//     //             "group1",
//     //             {
//     //                 std::make_shared<TypedSetting<int>>("a", 10),
//     //                 std::make_shared<TypedSetting<int>>("b", 20),
//     //                 std::make_shared<TypedSetting<int>>("c", 30)
//     //             }
//     //         ),
//     //         GroupConfig(
//     //             "group2",
//     //             {
//     //                 std::make_shared<TypedSetting<int>>("x", 100),
//     //                 std::make_shared<TypedSetting<int>>("y", 200),
//     //                 std::make_shared<TypedSetting<int>>("z", 300)
//     //             }
//     //         )
//     //     }
//     // };

//     // // Allocate buffer
//     // std::vector<char> buffer(cfg.serializationSize());

//     // cfg.serialize(buffer.data());



// 	// OutputV2::LEDStrip stripLeft(OutputV2::LEDManifest<WS2815, 5, RGB>{}, 8 * 24);
// 	// OutputV2::LEDStrip stripRight(OutputV2::LEDManifest<WS2815, 6, RGB>{}, 8 * 24);

// 	// OutputV2::LEDGrid ledgrid{16, 24};
// 	// ledgrid.addLEDStrip(stripLeft,
// 	// 	OutputV2::Range{0, 8},
// 	// 	OutputV2::Range{0, 24});
// 	// ledgrid.addLEDStrip(stripRight,
// 	// 	OutputV2::Range{8, 16},
// 	// 	OutputV2::Range{0, 24});


// 	// OutputV2::VirtualGrid vgrid{ledgrid,
// 	// 	{
// 	// 	OutputV2::Range{0, 16}, 2,
// 	// 	OutputV2::Range{0, 24}, 2
// 	// }};

// 	// vgrid.setColor(2, 6, OutputV2::Color{255, 255, 0});

// 	// OutputV2::HardwareLEDStrip strip_1(OutputV2::LEDManifest<WS2815, 5, RGB>{}, 8 * 24);
// 	// OutputV2::HardwareLEDStrip strip_2(OutputV2::LEDManifest<WS2815, 6, RGB>{}, 8 * 24);

// 	// CoreV2::LEDStripMapping mapping_1{
// 	// 	&strip_1,
// 	// 	CoreV2::Range{0, 4},
// 	// 	2,
// 	// 	CoreV2::Range{0, 12},
// 	// 	2
// 	// };

// 	// CoreV2::LEDStripMapping mapping_2{
// 	// 	&strip_2,
// 	// 	CoreV2::Range{4, 4},
// 	// 	2,
// 	// 	CoreV2::Range{0, 12},
// 	// 	2
// 	// };

// 	// CoreV2::Grid grid(CoreV2::Grid::Config{8, 12});
// 	// CoreV2::Grid grid;

// 	// grid.addMapping(mapping_1);
// 	// grid.addMapping(mapping_2);
// 	// After this point all entries of 8*12 grid are mapped to appropriate update function.


// 	// uint8_t* data_1;
// 	// CoreV2::UniverseUpdate update_1{1, data_1, CoreV2::Range{0, 4}, CoreV2::Range{0, 12}};
// 	// uint8_t* data_2;
// 	// CoreV2::UniverseUpdate update_2{2, data_2, CoreV2::Range{4, 8}, CoreV2::Range{0, 12}};

// 	// grid.update(update_1);
// 	// grid.update(update_2);

// 	// Engine::instance().addFixture<Christmas<6>>(true);
// 	// Engine::instance().addFixture<Christmas<9>>(true);
// 	Engine::instance().addFixture<Strip<60>>(true);
// 	Engine::instance().init();
// }

// void loop()
// {
// 	vTaskDelete(NULL);
// }













#include "Fixtures/All.h"
#include "Core/Engine.h"

void setup()
{
	Engine::instance().addFixture<Fixtures::Strip2815<60>>(true);
	Engine::instance().init();
}

void loop()
{
	vTaskDelete(NULL);
}


