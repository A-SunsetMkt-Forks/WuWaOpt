add_library(CombinationTweaker.lib CombinationTweaker.hpp CombinationTweaker.cpp)
add_library(CombinationMetaCache.lib CombinationMetaCache.hpp CombinationMetaCache.cpp)

target_link_libraries(CombinationMetaCache.lib CombinationTweaker.lib CharacterConfig.lib FullStats.lib Backpack.lib EffectiveStats.lib)
target_link_libraries(CombinationTweaker.lib CombinationMetaCache.lib SubStatRollConfig.lib Loca.lib UIConfig.lib magic_enum::magic_enum)
