//
// Created by EMCJava on 8/5/2024.
//

#include "CharacterPage.hpp"

#include <Opt/UI/UIConfig.hpp>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <imgui-SFML.h>
#include <imgui_internal.h>

#include <Common/ImGuiUtil.hxx>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <ranges>

namespace ImGui
{

ImTextureID
convertGLTextureHandleToImTextureID( auto glTextureHandle )
{
    return static_cast<ImTextureID>( glTextureHandle );
}

void
RotatedImage( const sf::Texture& texture, const sf::Vector2f& size, const sf::Vector2f& center,
              const sf::Color& tintColor = sf::Color::White )
{
    ImTextureID textureID = convertGLTextureHandleToImTextureID( texture.getNativeHandle( ) );

    ImGuiWindow* window = GetCurrentWindow( );

    const auto Rect = ImVec2 { size.x, size.y } * 1.414213562f;

    const ImRect bb( center - Rect / 2, center + Rect / 2 );
    ItemSize( bb );
    SetCursorScreenPos( bb.Min );

    if ( !ItemAdd( bb, 0 ) )
        return;

    const ImVec2 pos[ 4 ] =
        {
            center + ImRotate( ImVec2( -size.x * 0.5f, -size.y * 0.5f ), 0.707106781f, 0.707106781f ),
            center + ImRotate( ImVec2( +size.x * 0.5f, -size.y * 0.5f ), 0.707106781f, 0.707106781f ),
            center + ImRotate( ImVec2( +size.x * 0.5f, +size.y * 0.5f ), 0.707106781f, 0.707106781f ),
            center + ImRotate( ImVec2( -size.x * 0.5f, +size.y * 0.5f ), 0.707106781f, 0.707106781f ) };
    constexpr ImVec2 uvs[ 4 ] =
        {
            ImVec2( 0.0f, 0.0f ),
            ImVec2( 1.0f, 0.0f ),
            ImVec2( 1.0f, 1.0f ),
            ImVec2( 0.0f, 1.0f ) };

    window->DrawList->AddImageQuad( textureID, pos[ 0 ], pos[ 1 ], pos[ 2 ], pos[ 3 ], uvs[ 0 ], uvs[ 1 ], uvs[ 2 ], uvs[ 3 ], IM_COL32( tintColor.r, tintColor.g, tintColor.b, tintColor.a ) );
}

void
ImageRotatedFrame( const sf::Texture& texture, const sf::Vector2f& size, const sf::Vector2f& center,
                   const sf::Color& tintColor = sf::Color::White )
{
    ImTextureID textureID = convertGLTextureHandleToImTextureID( texture.getNativeHandle( ) );

    ImGuiWindow* window = GetCurrentWindow( );

    const auto Rect = ImVec2 { size.x, size.y } * 1.414213562f;

    const ImRect bb( center - Rect / 2, center + Rect / 2 );
    ItemSize( bb );
    SetCursorScreenPos( bb.Min );

    if ( !ItemAdd( bb, 0 ) )
        return;

    const ImVec2 pos[ 4 ] =
        {
            center + ImRotate( ImVec2( -size.x * 0.5f, -size.y * 0.5f ), 0.707106781f, 0.707106781f ),
            center + ImRotate( ImVec2( +size.x * 0.5f, -size.y * 0.5f ), 0.707106781f, 0.707106781f ),
            center + ImRotate( ImVec2( +size.x * 0.5f, +size.y * 0.5f ), 0.707106781f, 0.707106781f ),
            center + ImRotate( ImVec2( -size.x * 0.5f, +size.y * 0.5f ), 0.707106781f, 0.707106781f ) };
    constexpr ImVec2 uvs[ 4 ] =
        {
            ImVec2( 0.5f, 0.0f ),
            ImVec2( 1.0f, 0.5f ),
            ImVec2( 0.5f, 1.0f ),
            ImVec2( 0.0f, 0.5f ) };

    window->DrawList->AddImageQuad( textureID, pos[ 0 ], pos[ 1 ], pos[ 2 ], pos[ 3 ], uvs[ 0 ], uvs[ 1 ], uvs[ 2 ], uvs[ 3 ], IM_COL32( tintColor.r, tintColor.g, tintColor.b, tintColor.a ) );
}


}   // namespace ImGui

#define SAVE_CONFIG( x ) \
    if ( x ) SaveActiveCharacter( );

void
CharacterPage::DisplayStatConfigPopup( float WidthPerPanel )
{
    const auto   MainViewport = ImGui::GetMainViewport( );
    const ImVec2 Center       = MainViewport->GetCenter( );
    ImGui::SetNextWindowPos( Center, ImGuiCond_Always, ImVec2( 0.5f, 0.5f ) );
    if ( ImGui::BeginPopupModal( LanguageProvider[ "StatsComposition" ], nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        ImGui::SetNextWindowSizeConstraints( ImVec2 { 0, 0 }, { MainViewport->Size.x * 0.75f, FLT_MAX } );
        ImGui::BeginChild( "StatsCompositionList", ImVec2( 0, 0 ), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar );

        float PenalHeight = 250;
        for ( auto& [ Enabled, CompositionName, CompositionStats, CompositionDeepenStats ] : m_ActiveCharacterConfig->GetStatsCompositions( ) )
        {
            ImGui::SameLine( );
            ImGui::BeginChild( reinterpret_cast<uint64_t>( &CompositionName ), ImVec2( WidthPerPanel, 0 ), ImGuiChildFlags_AutoResizeY );
            ImGui::PushID( &CompositionName );

            const auto CompositionNameTextHeight = ImGui::CalcTextSize( CompositionName.c_str( ) ).y;
            ImGui::SeparatorTextEx( 0, CompositionName.c_str( ), CompositionName.data( ) + CompositionName.size( ), CompositionNameTextHeight + ImGui::GetStyle( ).SeparatorTextPadding.x );
            ImGui::SameLine( );
            if ( ImGui::ImageButton( "StatToggleImageButton", *UIConfig::GetTextureOrDefault( Enabled ? "ToggleOn" : "ToggleOff" ), { CompositionNameTextHeight, CompositionNameTextHeight } ) )
            {
                Enabled = !Enabled;
                SaveActiveCharacter( );
            }

            if ( !Enabled ) ImGui::BeginDisabled( );
            SAVE_CONFIG( ImGui::InputText( LanguageProvider[ "CompositionName" ], &CompositionName ) );
            ImGui::Separator( );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "FlatHealth" ], &CompositionStats.flat_health, 1, 0, 0, "%.0f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "FlatAttack" ], &CompositionStats.flat_attack, 1, 0, 0, "%.0f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "FlatDefence" ], &CompositionStats.flat_defence, 1, 0, 0, "%.0f" ) )
            ImGui::Separator( );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "Health%" ], &CompositionStats.percentage_health, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "Attack%" ], &CompositionStats.percentage_attack, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "Defence%" ], &CompositionStats.percentage_defence, 0.01, 0, 0, "%.2f" ) )
            ImGui::Separator( );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "ElementBuff%" ], &CompositionStats.buff_multiplier, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "AutoAttack%" ], &CompositionStats.auto_attack_buff, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "HeavyAttack%" ], &CompositionStats.heavy_attack_buff, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "SkillDamage%" ], &CompositionStats.skill_buff, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "UltDamage%" ], &CompositionStats.ult_buff, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "Heal%" ], &CompositionStats.heal_buff, 0.01, 0, 0, "%.2f" ) )
            ImGui::Separator( );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "CritRate" ], &CompositionStats.crit_rate, 0.01, 0, 0, "%.2f" ) )
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "CritDamage" ], &CompositionStats.crit_damage, 0.01, 0, 0, "%.2f" ) )
            ImGui::Separator( );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "AutoDeep%" ], &CompositionDeepenStats.auto_attack_multiplier, 0.01, 0, 0, "%.2f" ) );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "HeavyDeep%" ], &CompositionDeepenStats.heavy_attack_multiplier, 0.01, 0, 0, "%.2f" ) );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "SkillDeep%" ], &CompositionDeepenStats.skill_multiplier, 0.01, 0, 0, "%.2f" ) );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "UltDeep%" ], &CompositionDeepenStats.ult_multiplier, 0.01, 0, 0, "%.2f" ) );
            SAVE_CONFIG( ImGui::DragFloat( LanguageProvider[ "RawDeep%" ], &CompositionDeepenStats.raw_damage_multiplier, 0.01, 0, 0, "%.2f" ) );
            if ( !Enabled ) ImGui::EndDisabled( );

            ImGui::PopID( );
            PenalHeight = std::max( PenalHeight, ImGui::GetCursorPos( ).y );
            ImGui::Spacing( );
            ImGui::EndChild( );
        }

        const auto& Style = ImGui::GetStyle( );

        ImGui::SameLine( );

        ImGui::BeginChild( "ResizeComposition", ImVec2 { 15, 0 } );

        static bool IsAppendCompositionHovered = false;
        ImGui::PushStyleColor( ImGuiCol_ChildBg, IsAppendCompositionHovered ? IM_COL32( 0, 204, 102, 230 ) : IM_COL32( 0, 255, 153, 230 ) );
        ImGui::BeginChild( "AppendComposition", ImVec2( 15, PenalHeight / 2 - Style.ItemSpacing.y ), ImGuiChildFlags_Border, ImGuiWindowFlags_NoDecoration );
        ImGui::EndChild( );
        ImGui::PopStyleColor( );

        IsAppendCompositionHovered = ImGui::IsItemHovered( );
        if ( ImGui::IsItemClicked( ) )
        {
            m_ActiveCharacterConfig->StatsCompositions.emplace_back( true, "#" + std::to_string( m_ActiveCharacterConfig->StatsCompositions.size( ) ) );
            SaveActiveCharacter( );
        }

        static bool IsPopCompositionHovered = false;
        ImGui::PushStyleColor( ImGuiCol_ChildBg, IsPopCompositionHovered ? IM_COL32( 204, 0, 102, 230 ) : IM_COL32( 255, 0, 153, 230 ) );
        ImGui::BeginChild( "PopComposition", ImVec2( 15, PenalHeight / 2 - Style.ItemSpacing.y ), ImGuiChildFlags_Border, ImGuiWindowFlags_NoDecoration );
        ImGui::EndChild( );
        ImGui::PopStyleColor( );

        IsPopCompositionHovered = ImGui::IsItemHovered( );
        if ( m_ActiveCharacterConfig->StatsCompositions.size( ) > 1 && ImGui::IsItemClicked( ) )
        {
            m_ActiveCharacterConfig->StatsCompositions.pop_back( );
            SaveActiveCharacter( );
        }

        ImGui::EndChild( );


        ImGui::EndChild( );

        ImGui::Spacing( );
        ImGui::Separator( );
        ImGui::Spacing( );

        if ( ImGui::Button( LanguageProvider[ "Done" ], ImVec2( -1, 0 ) ) )
        {
            ImGui::CloseCurrentPopup( );
        }
        ImGui::SetItemDefaultFocus( );

        ImGui::EndPopup( );
    }
}

CharacterPage::CharacterPage( Loca& LocaObj )
    : LanguageObserver( LocaObj )
    , m_ElementLabels( LocaObj,
                       { "FireDamage",
                         "AirDamage",
                         "IceDamage",
                         "ElectricDamage",
                         "DarkDamage",
                         "LightDamage" } )
    , m_FoundationLabels( LocaObj,
                          { "FoundationAttack",
                            "FoundationHealth",
                            "FoundationDefence" } )
{
    if ( std::filesystem::exists( CharacterFileName ) )
    {
        m_CharactersNode      = YAML::LoadFile( CharacterFileName );
        CharacterConfigCaches = m_CharactersNode
            | std::views::transform( []( const YAML::const_iterator ::value_type& Node ) {
                                    return std::make_pair( Node.first.as<std::string>( ), std::get<1>( Node ).as<CharacterConfig>( ) );
                                } )
            | std::ranges::to<std::unordered_map>( );
    }

    for ( const auto& entry : std::filesystem::directory_iterator( "data/character_img" ) )
    {
        if ( entry.is_regular_file( ) )
        {
            const constexpr auto DefaultCharacterSize = 256;

            const auto Name = entry.path( ).stem( ).string( );
            UIConfig::LoadTexture( "CharImg_" + Name, entry.path( ).string( ) );
            m_CharacterNames.push_back( Name );

            const constexpr double CharacterZoomFactor = 0.75;
            const constexpr int    SmallSize           = DefaultCharacterSize * CharacterZoomFactor;
            if ( auto SmallTexture =
                     UIConfig::LoadTexture( "SmallCharImg_" + Name, entry.path( ).string( ),
                                            {
                                                ( DefaultCharacterSize - SmallSize ) / 2,
                                                ( DefaultCharacterSize - SmallSize ) / 2,
                                                SmallSize,
                                                SmallSize,
                                            } ) )
            {
                SmallTexture.value( )->generateMipmap( );
                SmallTexture.value( )->setSmooth( true );
            }
        }
    }

    // Merge two name set
    std::ranges::for_each( m_CharactersNode,
                           [ this ]( const YAML::const_iterator ::value_type& Node ) {
                               const auto  Name    = Node.first.as<std::string>( );
                               const auto& Profile = Node.second[ "Profile" ];
                               if ( Profile )
                               {
                                   // Replace by user defined texture
                                   UIConfig::LoadTexture( "CharImg_" + Name, Profile.as<std::string>( ) );
                               }
                               if ( !std::ranges::contains( m_CharacterNames, Name ) )
                                   m_CharacterNames.push_back( Name );
                           } );

    if ( m_CharacterNames.empty( ) )
    {
        spdlog::error( "No character image found, character list empty" );
        system( "pause" );
        exit( 1 );
    }

    LoadCharacter( m_CharacterNames.front( ) );
}

std::vector<std::string>
CharacterPage::GetCharacterList( ) const
{
    return m_CharactersNode
        | std::views::transform( []( const YAML::const_iterator::value_type& Node ) {
               return Node.first.as<std::string>( );
           } )
        | std::ranges::to<std::vector>( );
}

CharacterConfig&
CharacterPage::GetCharacter( const std::string& CharacterName )
{
    auto CacheIt = CharacterConfigCaches.find( CharacterName );
    if ( CacheIt == CharacterConfigCaches.end( ) )
    {
        CacheIt = CharacterConfigCaches.emplace_hint( CacheIt, CharacterName, CharacterConfig { } );
        FromNode( YAML::Node { }, CacheIt->second );   // Load default config
        SaveCharacter( CharacterName );
    }

    return CacheIt->second;
}

void
CharacterPage::LoadCharacter( const std::string& CharacterName )
{
    m_ActiveCharacterConfig = &GetCharacter( m_ActiveCharacterName = CharacterName );
    m_ActiveCharacterConfig->UpdateOverallStats( );

    m_ActiveSkillDisplay = {
        .auto_attack_multiplier  = m_ActiveCharacterConfig->SkillConfig.auto_attack_multiplier * 100,
        .heavy_attack_multiplier = m_ActiveCharacterConfig->SkillConfig.heavy_attack_multiplier * 100,
        .skill_multiplier        = m_ActiveCharacterConfig->SkillConfig.skill_multiplier * 100,
        .ult_multiplier          = m_ActiveCharacterConfig->SkillConfig.ult_multiplier * 100,
        .heal_multiplier         = m_ActiveCharacterConfig->SkillConfig.heal_multiplier * 100,
        .raw_damage_multiplier   = m_ActiveCharacterConfig->SkillConfig.raw_damage_multiplier * 100 };
    m_ActiveDeepenDisplay = {
        .auto_attack_multiplier  = m_ActiveCharacterConfig->CharacterOverallDeepenStats.auto_attack_multiplier * 100,
        .heavy_attack_multiplier = m_ActiveCharacterConfig->CharacterOverallDeepenStats.heavy_attack_multiplier * 100,
        .skill_multiplier        = m_ActiveCharacterConfig->CharacterOverallDeepenStats.skill_multiplier * 100,
        .ult_multiplier          = m_ActiveCharacterConfig->CharacterOverallDeepenStats.ult_multiplier * 100,
        .heal_multiplier         = m_ActiveCharacterConfig->CharacterOverallDeepenStats.heal_multiplier * 100,
        .raw_damage_multiplier   = m_ActiveCharacterConfig->CharacterOverallDeepenStats.raw_damage_multiplier * 100 };
}

void
CharacterPage::SaveCharacter( const std::string& CharacterName )
{
    auto CacheIt = CharacterConfigCaches.find( CharacterName );
    if ( CacheIt == CharacterConfigCaches.end( ) )
    {
        CacheIt = CharacterConfigCaches.emplace_hint( CacheIt, CharacterName, CharacterConfig { } );
        FromNode( YAML::Node { }, CacheIt->second );   // Load default config
    }

    CacheIt->second.InternalStageID++;

    // Assume all changes will lead to SaveActiveCharacter being called
    CacheIt->second.UpdateOverallStats( );

    std::ofstream OutFile( CharacterFileName );

    m_CharactersNode[ m_ActiveCharacterName ] = CacheIt->second;
    OutFile << m_CharactersNode;

    OutFile.close( );
}

void
CharacterPage::SaveActiveCharacter( )
{
    m_ActiveCharacterConfig->InternalStageID++;

    // Assume all changes will lead to SaveActiveCharacter being called
    m_ActiveCharacterConfig->UpdateOverallStats( );
    m_ActiveDeepenDisplay = {
        .auto_attack_multiplier  = m_ActiveCharacterConfig->CharacterOverallDeepenStats.auto_attack_multiplier * 100,
        .heavy_attack_multiplier = m_ActiveCharacterConfig->CharacterOverallDeepenStats.heavy_attack_multiplier * 100,
        .skill_multiplier        = m_ActiveCharacterConfig->CharacterOverallDeepenStats.skill_multiplier * 100,
        .ult_multiplier          = m_ActiveCharacterConfig->CharacterOverallDeepenStats.ult_multiplier * 100,
        .heal_multiplier         = m_ActiveCharacterConfig->CharacterOverallDeepenStats.heal_multiplier * 100,
        .raw_damage_multiplier   = m_ActiveCharacterConfig->CharacterOverallDeepenStats.raw_damage_multiplier * 100 };

    std::ofstream OutFile( CharacterFileName );

    m_CharactersNode[ m_ActiveCharacterName ] = *m_ActiveCharacterConfig;
    OutFile << m_CharactersNode;

    OutFile.close( );
}

bool
CharacterPage::DisplayCharacterInfo( float Width, float* HeightOut )
{

    bool        Changed = false;
    const auto& Style   = ImGui::GetStyle( );

    ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 5.0f );
    ImGui::BeginChild( "ConfigPanel", ImVec2( Width - Style.WindowPadding.x * 4, 0 ), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize );

    const auto DisplayImageWithSize = []( const std::string& ImageName, float ImageSize ) {
        const auto StartPos = ImGui::GetCursorPos( );
        const auto Texture  = UIConfig::GetTexture( "CharImg_" + ImageName );
        if ( Texture.has_value( ) )
        {
            ImGui::Image( *Texture.value( ), sf::Vector2f { ImageSize, ImageSize } );
        } else
        {
            ImGui::Image( *UIConfig::GetTextureDefault( ), sf::Vector2f { ImageSize, ImageSize } );
            ImGui::SetCursorPos( StartPos );
            ImGui::Text( "%s", ImageName.c_str( ) );
        }
    };

    {
        static float   ConfigHeight = 50;
        constexpr auto ImageSpace   = 20;

        ImGui::BeginChild( "StaticPanel##Config", ImVec2( ( Width - ConfigHeight - ImageSpace ) - Style.WindowPadding.x * 8, 0 ), ImGuiChildFlags_AutoResizeY );
        ImGui::SeparatorText( LanguageProvider[ "StaticConfig" ] );
        ImGui::PushItemWidth( -ImGui::CalcTextSize( LanguageProvider[ "ElResistance" ] ).x - Style.FramePadding.x );
        SAVE_CONFIG( ImGui::SliderInt( LanguageProvider[ "CharLevel" ], &m_ActiveCharacterConfig->CharacterLevel, 1, 90 ) )
        SAVE_CONFIG( ImGui::SliderInt( LanguageProvider[ "EnemyLevel" ], &m_ActiveCharacterConfig->EnemyLevel, 1, 90 ) )
        SAVE_CONFIG( ImGui::SliderFloat( LanguageProvider[ "ElResistance" ], &m_ActiveCharacterConfig->ElementResistance, 0, 1, "%.2f" ) )
        SAVE_CONFIG( ImGui::SliderFloat( LanguageProvider[ "DamReduction" ], &m_ActiveCharacterConfig->ElementDamageReduce, 0, 1, "%.2f" ) )
        ImGui::PopItemWidth( );
        ConfigHeight = ImGui::GetWindowHeight( );
        ImGui::EndChild( );

        ImGui::SameLine( 0, ImageSpace );

        ImGui::BeginChild( "StaticPanel##Image", ImVec2( ConfigHeight, ConfigHeight ), ImGuiChildFlags_AutoResizeY );
        {
            const auto ChildStartPos = ImGui::GetCursorPos( );
            if ( ImGui::Selectable( "##Click", false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick, ImVec2( ConfigHeight, ConfigHeight ) ) )
            {
                ImGui::OpenPopup( LanguageProvider[ "CharSel" ] );
            }
            ImGui::SetCursorPos( ChildStartPos );
            DisplayImageWithSize( m_ActiveCharacterName, ConfigHeight );
        }

        ImGui::SetNextWindowPos( ImGui::GetMainViewport( )->GetCenter( ), ImGuiCond_Appearing, ImVec2( 0.5f, 0.5f ) );
        if ( ImGui::BeginPopupModal( LanguageProvider[ "CharSel" ], nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize ) )
        {
            static constexpr auto CharacterImgSpacing = 20;
            static constexpr auto CharacterImgSize    = 150;

            ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2 { } );
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2 { } );
            ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2 { } );
            int X = 0, Y = 0;
            for ( int i = 0; i < m_CharacterNames.size( ); ++i )
            {
                auto& Name = m_CharacterNames[ i ];
                ImGui::PushID( std::hash<std::string>( )( "CharacterPick" ) + i );

                if ( X != 0 ) ImGui::SameLine( 0, CharacterImgSpacing );

                {
                    ImGui::BeginChild( "CharacterCard", ImVec2( CharacterImgSize, CharacterImgSize ), ImGuiChildFlags_Border );
                    const auto ChildStartPos = ImGui::GetCursorPos( );
                    if ( ImGui::Selectable( "##Click", m_ActiveCharacterName == Name, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick, ImVec2( CharacterImgSize, CharacterImgSize ) ) )
                    {
                        Changed = true;
                        LoadCharacter( m_ActiveCharacterName = Name );
                        ImGui::CloseCurrentPopup( );
                    }
                    ImGui::SetCursorPos( ChildStartPos );
                    DisplayImageWithSize( Name, CharacterImgSize );
                    ImGui::EndChild( );
                }

                ++X;
                if ( X >= 6 )
                {
                    Y++;
                    X = 0;
                    ImGui::ItemSize( ImVec2( 0, CharacterImgSpacing ) );
                }

                ImGui::PopID( );
            }
            ImGui::PopStyleVar( 3 );

            ImGui::Spacing( );
            ImGui::Separator( );
            ImGui::Spacing( );

            if ( ImGui::Button( LanguageProvider[ "Cancel" ], ImVec2( -1, 0 ) ) )
            {
                ImGui::CloseCurrentPopup( );
            }
            ImGui::SetItemDefaultFocus( );
            ImGui::EndPopup( );
        }

        ImGui::EndChild( );
    }

    ImGui::Separator( );

    const auto PanelWidth = Width / 2 - Style.WindowPadding.x * 4;
    ImGui::BeginChild( "ConfigPanel##Character", ImVec2( PanelWidth, 0 ), ImGuiChildFlags_AutoResizeY );

    const auto* OverallStatsText       = LanguageProvider[ "OverallStats" ];
    const auto  OverallStatsTextHeight = ImGui::CalcTextSize( OverallStatsText ).y;

    ImGui::SeparatorTextEx( 0, OverallStatsText, OverallStatsText + strlen( OverallStatsText ), OverallStatsTextHeight + Style.SeparatorTextPadding.x );
    ImGui::SameLine( );
    if ( ImGui::ImageButton( "DecompositionImageButton", *UIConfig::GetTextureOrDefault( "Decomposition" ), { OverallStatsTextHeight, OverallStatsTextHeight } ) )
    {
        ImGui::OpenPopup( LanguageProvider[ "StatsComposition" ] );
    }
    DisplayStatConfigPopup( 350 );

    ImGui::PushID( "OverallStats" );
    ImGui::BeginDisabled( );
    ImGui::DragFloat( LanguageProvider[ "FlatHealth" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).flat_health ), 1, 0, 0, "%.0f" );
    ImGui::DragFloat( LanguageProvider[ "FlatAttack" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).flat_attack ), 1, 0, 0, "%.0f" );
    ImGui::DragFloat( LanguageProvider[ "FlatDefence" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).flat_defence ), 1, 0, 0, "%.0f" );
    ImGui::DragFloat( LanguageProvider[ "Health%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).percentage_health ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "Attack%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).percentage_attack ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "Defence%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).percentage_defence ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "ElementBuff%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).buff_multiplier ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "AutoAttack%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).auto_attack_buff ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "HeavyAttack%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).heavy_attack_buff ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "SkillDamage%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).skill_buff ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "UltDamage%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).ult_buff ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "Heal%" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).heal_buff ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "CritRate" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).crit_rate ), 0.01, 0, 0, "%.2f" );
    ImGui::DragFloat( LanguageProvider[ "CritDamage" ], const_cast<float*>( &m_ActiveCharacterConfig->GetOverallStats( ).crit_damage ), 0.01, 0, 0, "%.2f" );
    ImGui::EndDisabled( );
    ImGui::PopID( );

    float PanelHeight = ImGui::GetCursorPosY( );
    ImGui::EndChild( );

    ImGui::SameLine( );

    {
        ImGui::BeginChild( "ConfigPanel##EchoesWrap", ImVec2( PanelWidth, std::clamp( PanelHeight, 1.f, FLT_MAX ) ) );
        ImGui::SeparatorText( LanguageProvider[ "EchoEquipped" ] );
        const auto EchoWrapPanelStart = ImGui::GetWindowPos( );

        PanelHeight -= ImGui::GetCursorScreenPos( ).y - EchoWrapPanelStart.y;
        ImGui::BeginChild( "ConfigPanel##Echoes", ImVec2( PanelWidth, PanelHeight ), ImGuiChildFlags_Border, ImGuiWindowFlags_NoDecoration );
        ImGui::PushID( "ConfigPanelEchoes" );

        const float AvailableWidth = PanelWidth;
        const auto  EchoPanelStart = ImGui::GetWindowPos( );
        const auto  EchoXSpan      = AvailableWidth / 5;
        const auto  EchoImageSize  = sf::Vector2f { EchoXSpan, EchoXSpan } / 1.2;
        const auto  EchoFrameSize  = EchoImageSize * 1.4;

        const std::array<sf::Vector2f, 5> EchoCenters {
            EchoPanelStart + sf::Vector2f {    AvailableWidth / 2,     PanelHeight / 2},
            EchoPanelStart + sf::Vector2f {    AvailableWidth / 4,     PanelHeight / 4},
            EchoPanelStart + sf::Vector2f {3 * AvailableWidth / 4,     PanelHeight / 4},
            EchoPanelStart + sf::Vector2f {    AvailableWidth / 4, 3 * PanelHeight / 4},
            EchoPanelStart + sf::Vector2f {3 * AvailableWidth / 4, 3 * PanelHeight / 4}
        };

        bool ErasedThisFrame = false;
        for ( int i = 0; i < 5; i++ )
        {
            if ( m_ActiveCharacterConfig->EquippedEchoHashes.size( ) > i )
            {
                ImGui::ImageRotatedFrame(
                    *UIConfig::GetTextureOrDefault( m_ActiveCharacterConfig->RuntimeEquippedEchoName[ i ] ),
                    EchoImageSize,
                    EchoCenters[ i ] );
                uint8_t FrameAlpha = 70;
                if ( ImGui::IsItemHovered( ) ) FrameAlpha = 150;
                ImGui::RotatedImage( *UIConfig::GetTextureOrDefault( "EchoFrame" ), EchoFrameSize, EchoCenters[ i ], sf::Color { 255, 255, 255, FrameAlpha } );
                if ( !ErasedThisFrame && ImGui::IsItemClicked( ) && ImGui::GetMouseClickedCount( ImGuiMouseButton_Left ) == 2 )
                {
                    m_ActiveCharacterConfig->EquippedEchoHashes.erase( m_ActiveCharacterConfig->EquippedEchoHashes.begin( ) + i );
                    m_ActiveCharacterConfig->RuntimeEquippedEchoName.erase( m_ActiveCharacterConfig->RuntimeEquippedEchoName.begin( ) + i );
                    SaveActiveCharacter( );
                    ErasedThisFrame = true;
                }
            } else
            {
                ImGui::RotatedImage( *UIConfig::GetTextureOrDefault( "EchoFrame" ), EchoImageSize, EchoCenters[ i ] );
            }
        }

        ImGui::PopID( );
        ImGui::EndChild( );
        ImGui::EndChild( );
    }

    ImGui::NewLine( );
    ImGui::Separator( );
    ImGui::NewLine( );

#define SAVE_MULTIPLIER_CONFIG( name, type, stat )                                                                 \
    if ( ImGui::DragFloat( name, &m_Active##type##Display.stat##_multiplier ) )                                    \
    {                                                                                                              \
        m_ActiveCharacterConfig->type##Config.stat##_multiplier = m_Active##type##Display.stat##_multiplier / 100; \
        SaveActiveCharacter( );                                                                                    \
    }

    {
        ImGui::BeginChild( "ConfigPanel##Deepen", ImVec2( Width / 2 - Style.WindowPadding.x * 4, 0 ), ImGuiChildFlags_AutoResizeY );
        ImGui::SeparatorText( LanguageProvider[ "DMGDeep%" ] );
        ImGui::PushID( "Deepen" );
        ImGui::BeginDisabled( );
        ImGui::DragFloat( LanguageProvider[ "AutoTotal%" ], &m_ActiveDeepenDisplay.auto_attack_multiplier );
        ImGui::DragFloat( LanguageProvider[ "HeavyTotal%" ], &m_ActiveDeepenDisplay.heavy_attack_multiplier );
        ImGui::DragFloat( LanguageProvider[ "SkillTotal%" ], &m_ActiveDeepenDisplay.skill_multiplier );
        ImGui::DragFloat( LanguageProvider[ "UltTotal%" ], &m_ActiveDeepenDisplay.ult_multiplier );
        ImGui::DragFloat( LanguageProvider[ "RDMGTotal%" ], &m_ActiveDeepenDisplay.raw_damage_multiplier );
        ImGui::EndDisabled( );
        ImGui::PopID( );
        ImGui::EndChild( );

        ImGui::SameLine( );

        ImGui::BeginChild( "ConfigPanel##Skill", ImVec2( Width / 2 - Style.WindowPadding.x * 4, 0 ), ImGuiChildFlags_AutoResizeY );
        ImGui::SeparatorText( LanguageProvider[ "CycleTotal%" ] );
        ImGui::PushID( "Skill" );
        SAVE_MULTIPLIER_CONFIG( LanguageProvider[ "AutoTotal%" ], Skill, auto_attack )
        SAVE_MULTIPLIER_CONFIG( LanguageProvider[ "HeavyTotal%" ], Skill, heavy_attack )
        SAVE_MULTIPLIER_CONFIG( LanguageProvider[ "SkillTotal%" ], Skill, skill )
        SAVE_MULTIPLIER_CONFIG( LanguageProvider[ "UltTotal%" ], Skill, ult )
        SAVE_MULTIPLIER_CONFIG( LanguageProvider[ "HealTotal%" ], Skill, heal )

        SAVE_MULTIPLIER_CONFIG( LanguageProvider[ "RDMGTotal%" ], Skill, raw_damage )
        ImGui::SameLine( );
        ImGui::TextDisabled( "(?)" );
        if ( ImGui::BeginItemTooltip( ) )
        {
            ImGui::PushTextWrapPos( ImGui::GetFontSize( ) * 35.0f );
            ImGui::TextUnformatted( LanguageProvider[ "RDMGExplain" ] );
            ImGui::PopTextWrapPos( );
            ImGui::EndTooltip( );
        }

        ImGui::PopID( );
        ImGui::EndChild( );
    }
#undef SAVE_MULTIPLIER_CONFIG

    ImGui::NewLine( );
    ImGui::Separator( );
    ImGui::NewLine( );

    ImGui::PushItemWidth( -200 );

    ImGui::BeginChild( "ConfigPanel##ElementType", ImVec2( Width / 2 - Style.WindowPadding.x * 4, 0 ), ImGuiChildFlags_AutoResizeY );
    if ( ImGui::Combo( LanguageProvider[ "ElementType" ],
                       (int*) &m_ActiveCharacterConfig->CharacterElement,
                       m_ElementLabels.GetRawStrings( ),
                       m_ElementLabels.GetStringCount( ) ) )
    {
        SaveActiveCharacter( );
        Changed = true;
    }

    ImGui::EndChild( );

    ImGui::SameLine( );

    ImGui::BeginChild( "ConfigPanel##FoundationType", ImVec2( Width / 2 - Style.WindowPadding.x * 4, 0 ), ImGuiChildFlags_AutoResizeY );

    SAVE_CONFIG( ImGui::Combo( LanguageProvider[ "FoundationType" ],
                               (int*) &m_ActiveCharacterConfig->CharacterStatsFoundation,
                               m_FoundationLabels.GetRawStrings( ),
                               m_FoundationLabels.GetStringCount( ) ) )
    ImGui::EndChild( );

    // ImGui::PopItemWidth( );

    if ( HeightOut ) *HeightOut = ImGui::GetWindowHeight( );
    ImGui::EndChild( );

    return Changed;
}
