/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file MapperBuilderTest.cpp
 *   Unit tests for run-time mapper object building functionality.
 **************************************************************************************************/

#include "TestCase.h"

#include "MapperBuilder.h"

#include <memory>
#include <optional>
#include <set>
#include <string_view>

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "ForceFeedbackTypes.h"
#include "Mapper.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;
  using ::Xidi::Controller::EAxis;
  using ::Xidi::Controller::ForceFeedback::EActuatorMode;
  using ::Xidi::Controller::ForceFeedback::SActuatorElement;

  /// Verifies that the two supplied element mappers are equivalent to one another and flags a test
  /// failure if not. Only works for simple element mappers that uniquely target zero or one
  /// specific controller elements and have no side effects.
  /// @param [in] elementMapperA One side of the comparison.
  /// @param [in] elementMapperB Another side of the comparison.
  static void VerifyElementMappersAreEquivalent(
      const IElementMapper& elementMapperA, const IElementMapper& elementMapperB)
  {
    TEST_ASSERT(elementMapperA.GetTargetElementCount() == elementMapperB.GetTargetElementCount());

    for (int i = 0; i < elementMapperA.GetTargetElementCount(); ++i)
      TEST_ASSERT(elementMapperA.GetTargetElementAt(i) == elementMapperB.GetTargetElementAt(i));
  }

  /// Verifies that the two supplied element maps are equivalent to one another and flags a test
  /// failure if not. Only works for simple element mappers that uniquely target zero or one
  /// specific controller elements and have no side effects.
  /// @param [in] elementMapA One side of the comparison.
  /// @param [in] elementMapB Another side of the comparison.
  static void VerifyElementMapsAreEquivalent(
      const Mapper::UElementMap& elementMapA, const Mapper::UElementMap& elementMapB)
  {
    for (unsigned int i = 0; i < _countof(Mapper::UElementMap::all); ++i)
    {
      if (nullptr != elementMapA.all[i])
      {
        TEST_ASSERT(nullptr != elementMapB.all[i]);
        VerifyElementMappersAreEquivalent(*elementMapA.all[i], *elementMapB.all[i]);
      }
      else
      {
        TEST_ASSERT(nullptr == elementMapB.all[i]);
      }
    }
  }

  /// Verifies that the two supplied force feedback actuator maps are equivalent to one another and
  /// flags a test failure if not.
  /// @param [in] actuatorMapA One side of the comparison.
  /// @param [in] actuatorMapB Another side of the comparison.
  static void VerifyForceFeedbackActuatorMapsAreEquivalent(
      const Mapper::UForceFeedbackActuatorMap& actuatorMapA,
      const Mapper::UForceFeedbackActuatorMap& actuatorMapB)
  {
    for (unsigned int i = 0; i < _countof(Mapper::UForceFeedbackActuatorMap::all); ++i)
      TEST_ASSERT(actuatorMapA.all[i] == actuatorMapB.all[i]);
  }

  /// Verifies that the supplied element map is empty and flags a test failure if not.
  static void VerifyElementMapIsEmpty(const Mapper::UElementMap& elementMapToCheck)
  {
    for (const auto& elementMapper : elementMapToCheck.all)
      TEST_ASSERT(elementMapper == nullptr);
  }

  /// Verifies that the specified element map blueprint specification matches a test specification.
  /// Test specification consists of a searchable container holding element map indices, all of
  /// which hold an element mapper equivalent to the supplied element mapper. Only works for simple
  /// element mappers that uniquely target zero or one specific controller elements and have no side
  /// effects. Any element map indices not present in the container are expected to be empty. A test
  /// failure is flagged if a mismatch is found.
  /// @tparam StdIndexContainer Standard searchable container type holding the element map indices
  /// to check. Any indices less than 0 are ignored.
  /// @param [in] elementMapLayout Container holding the element map indices and acting as a layout
  /// descriptor.
  /// @param [in] elementMapper Element mapper to be used for equivalence checking at each index of
  /// the layout descriptor.
  /// @param [in] elementMapSpecToCheck Element map descriptor object being checked for matching
  /// with the expected spec.
  template <typename StdIndexContainer> static void VerifyElementMapSpecMatchesSpec(
      const StdIndexContainer& elementMapLayout,
      const IElementMapper& elementMapper,
      const MapperBuilder::TElementMapSpec& elementMapSpecToCheck)
  {
    for (unsigned int i = 0; i < _countof(Mapper::UElementMap::all); ++i)
    {
      if (elementMapLayout.contains(i))
      {
        TEST_ASSERT(true == elementMapSpecToCheck.contains(i));
        VerifyElementMappersAreEquivalent(elementMapper, *elementMapSpecToCheck.at(i));
      }
      else
      {
        TEST_ASSERT(false == elementMapSpecToCheck.contains(i));
      }
    }
  }

  /// Verifies that the specified element map matches a test specification.
  /// Test specification consists of a searchable container holding element map indices, all of
  /// which hold an element mapper equivalent to the supplied element mapper. Only works for simple
  /// element mappers that uniquely target zero or one specific controller elements and have no side
  /// effects. Any element map indices not present in the container are expected to be empty. A test
  /// failure is flagged if a mismatch is found.
  /// @tparam StdIndexContainer Standard searchable container type holding the element map indices
  /// to check. Any indices less than 0 are ignored.
  /// @param [in] elementMapLayout Container holding the element map indices and acting as a layout
  /// descriptor.
  /// @param [in] elementMapper Element mapper to be used for equivalence checking at each index of
  /// the layout descriptor.
  /// @param [in] elementMapToCheck Element map object being checked for matching with the spec.
  template <typename StdIndexContainer> static void VerifyElementMapMatchesSpec(
      const StdIndexContainer& elementMapLayout,
      const IElementMapper& elementMapper,
      const Mapper::UElementMap& elementMapToCheck)
  {
    for (unsigned int i = 0; i < _countof(Mapper::UElementMap::all); ++i)
    {
      if (elementMapLayout.contains(i))
      {
        TEST_ASSERT(nullptr != elementMapToCheck.all[i]);
        VerifyElementMappersAreEquivalent(elementMapper, *elementMapToCheck.all[i]);
      }
      else
      {
        TEST_ASSERT(nullptr == elementMapToCheck.all[i]);
      }
    }
  }

  // Verifies that blueprints can be created and successfully identified.
  TEST_CASE(MapperBuilder_BlueprintName_Nominal)
  {
    constexpr std::wstring_view kMapperNames[] = {
        L"TestMapper", L"testMapper", L"TestMapper2", L"testMapper2"};

    MapperBuilder builder;

    for (auto mapperName : kMapperNames)
      TEST_ASSERT(false == builder.DoesBlueprintNameExist(mapperName));

    for (auto mapperName : kMapperNames)
      TEST_ASSERT(true == builder.CreateBlueprint(mapperName));

    for (auto mapperName : kMapperNames)
      TEST_ASSERT(true == builder.DoesBlueprintNameExist(mapperName));
  }

  // Verifies that attempts to create blueprints with the same name are rejected.
  TEST_CASE(MapperBuilder_BlueprintName_DuplicatesRejected)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr int kRepeatTimes = 10;

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (int i = 0; i < kRepeatTimes; ++i)
    {
      TEST_ASSERT(false == builder.CreateBlueprint(kMapperName));
      TEST_ASSERT(true == builder.DoesBlueprintNameExist(kMapperName));
    }
  }

  // Verifies that attempts to create blueprints with the same name as existing mapper objects are
  // rejected. This test uses the names of known documented mappers.
  TEST_CASE(MapperBuilder_BlueprintName_ExistingMapperNameRejected)
  {
    constexpr std::wstring_view kMapperNames[] = {
        L"StandardGamepad",
        L"DigitalGamepad",
        L"ExtendedGamepad",
        L"XInputNative",
        L"XInputSharedTriggers"};

    MapperBuilder builder;

    for (auto mapperName : kMapperNames)
      TEST_ASSERT(false == builder.DoesBlueprintNameExist(mapperName));

    for (auto mapperName : kMapperNames)
      TEST_ASSERT(false == builder.CreateBlueprint(mapperName));
  }

  // Verifies that new blueprints are empty upon creation.
  TEST_CASE(MapperBuilder_CreateBlueprint_Empty)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    const MapperBuilder::TElementMapSpec* const kElementMapSpec =
        builder.GetBlueprintElementMapSpec(kMapperName);
    TEST_ASSERT(nullptr != kElementMapSpec);
    TEST_ASSERT(true == kElementMapSpec->empty());

    const std::optional<std::wstring_view> maybeTemplateName =
        builder.GetBlueprintTemplate(kMapperName);
    TEST_ASSERT(true == maybeTemplateName.has_value());
    TEST_ASSERT(true == maybeTemplateName.value().empty());
  }

  // Verifies that element mappers can be set in the nominal case of valid controller elements being
  // specified.
  TEST_CASE(MapperBuilder_ElementMap_Nominal)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr AxisMapper kTestElementMapper(EAxis::X);
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(stickLeftY), ELEMENT_MAP_INDEX_OF(triggerLT)};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(
          true ==
          builder.SetBlueprintElementMapper(
              kMapperName, controllerElement, kTestElementMapper.Clone()));

    const MapperBuilder::TElementMapSpec* const kElementMapSpec =
        builder.GetBlueprintElementMapSpec(kMapperName);
    TEST_ASSERT(nullptr != kElementMapSpec);
    VerifyElementMapSpecMatchesSpec(kControllerElements, kTestElementMapper, *kElementMapSpec);
  }

  // Verifies that element mappers can be set and then cleared, leading to an empty element map
  // specification.
  TEST_CASE(MapperBuilder_ElementMap_EmptyAfterSetAndClear)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr AxisMapper kTestElementMapper(EAxis::X);
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(stickLeftY), ELEMENT_MAP_INDEX_OF(triggerLT)};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto controllerElement : kControllerElements)
    {
      TEST_ASSERT(
          true ==
          builder.SetBlueprintElementMapper(
              kMapperName, controllerElement, kTestElementMapper.Clone()));
      TEST_ASSERT(true == builder.ClearBlueprintElementMapper(kMapperName, controllerElement));
    }

    const MapperBuilder::TElementMapSpec* const kElementMapSpec =
        builder.GetBlueprintElementMapSpec(kMapperName);
    TEST_ASSERT(nullptr != kElementMapSpec);
    TEST_ASSERT(true == kElementMapSpec->empty());
  }

  // Similar to the nominal case but with the addition of clear attempts which fail.
  TEST_CASE(MapperBuilder_ElementMap_IneffectiveClearNoEffect)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr AxisMapper kTestElementMapper(EAxis::X);
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(stickLeftY), ELEMENT_MAP_INDEX_OF(triggerLT)};
    constexpr int kControllerElementsToClear[] = {
        ELEMENT_MAP_INDEX_OF(stickLeftX),
        ELEMENT_MAP_INDEX_OF(stickRightY),
        ELEMENT_MAP_INDEX_OF(dpadLeft),
        ELEMENT_MAP_INDEX_OF(triggerRT),
        ELEMENT_MAP_INDEX_OF(buttonBack)};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(
          true ==
          builder.SetBlueprintElementMapper(
              kMapperName, controllerElement, kTestElementMapper.Clone()));

    for (auto controllerElementToClear : kControllerElementsToClear)
      TEST_ASSERT(
          false == builder.ClearBlueprintElementMapper(kMapperName, controllerElementToClear));

    const MapperBuilder::TElementMapSpec* const kElementMapSpec =
        builder.GetBlueprintElementMapSpec(kMapperName);
    TEST_ASSERT(nullptr != kElementMapSpec);
    VerifyElementMapSpecMatchesSpec(kControllerElements, kTestElementMapper, *kElementMapSpec);
  }

  // Verifies that element mappers can be set with some being valid and some being invalid.
  TEST_CASE(MapperBuilder_ElementMap_SomeInvalid)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr AxisMapper kTestElementMapper(EAxis::X);

    // Same as above, but with negative indices to indicate invalid controller elements.
    // The condition for successful insertion uses comparison-with-0 to decide whether to expect
    // success or failure. Similarly, the loop that verifies null vs non-null element mappers skips
    // over all indices less than 0.
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(stickLeftY), -1, ELEMENT_MAP_INDEX_OF(triggerLT), -2};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(
          (controllerElement >= 0) ==
          builder.SetBlueprintElementMapper(
              kMapperName, controllerElement, kTestElementMapper.Clone()));

    const MapperBuilder::TElementMapSpec* const kElementMapSpec =
        builder.GetBlueprintElementMapSpec(kMapperName);
    TEST_ASSERT(nullptr != kElementMapSpec);
    VerifyElementMapSpecMatchesSpec(kControllerElements, kTestElementMapper, *kElementMapSpec);
  }

  // Verifies that element mappers cannot be set on unknown mappers.
  // The element mappers themselves are valid, but the mapper name is unknown.
  TEST_CASE(MapperBuilder_ElementMap_UnknownMapper)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr std::wstring_view kUnknownMapperName = L"UnknownMapper";
    constexpr AxisMapper kTestElementMapper(EAxis::X);
    const std::wstring_view kControllerElements[] = {L"StickLeftY", L"TriggerLT"};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(
          false ==
          builder.SetBlueprintElementMapper(
              kUnknownMapperName, controllerElement, kTestElementMapper.Clone()));

    TEST_ASSERT(nullptr == builder.GetBlueprintElementMapSpec(kUnknownMapperName));

    const MapperBuilder::TElementMapSpec* const kElementMapSpec =
        builder.GetBlueprintElementMapSpec(kMapperName);
    TEST_ASSERT(nullptr != kElementMapSpec);
    TEST_ASSERT(true == kElementMapSpec->empty());
  }

  // Verifies that template names can be set regardless of whether or not they refer to existing
  // mappers, mapper blueprints, or even the mapper blueprint itself. These should all be successful
  // because template names are not checked for semantic correctness until an attempt is made to
  // construct a mapper object.
  TEST_CASE(MapperBuilder_TemplateName_Nominal)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr std::wstring_view kTemplateNames[] = {
        kMapperName, L"RandomMapper", L"StandardGamepad"};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto templateName : kTemplateNames)
    {
      TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, templateName));
      TEST_ASSERT(true == builder.GetBlueprintTemplate(kMapperName).has_value());
      TEST_ASSERT(templateName == builder.GetBlueprintTemplate(kMapperName).value());
    }
  }

  // Verifies that template name setting attempts are rejected if the mapper name is unknown.
  TEST_CASE(MapperBuilder_TemplateName_UnknownMapper)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr std::wstring_view kUnknownMapperName = L"UnknownMapper";
    constexpr std::wstring_view kTemplateNames[] = {
        kMapperName, L"RandomMapper", L"StandardGamepad"};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto templateName : kTemplateNames)
    {
      TEST_ASSERT(false == builder.SetBlueprintTemplate(kUnknownMapperName, templateName));
      TEST_ASSERT(false == builder.GetBlueprintTemplate(kUnknownMapperName).has_value());
    }

    TEST_ASSERT(true == builder.GetBlueprintTemplate(kMapperName).has_value());
    TEST_ASSERT(true == builder.GetBlueprintTemplate(kMapperName).value().empty());
  }

  // Verifies that an empty mapper can be built and registered. This is the trivial case.
  // Element map is expected to be empty.
  TEST_CASE(MapperBuilder_Build_NoTemplate_Trivial)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);
    TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());
    VerifyElementMapIsEmpty(mapper->ElementMap());
  }

  // Verifies that a simple mapper without a template can be built and registered.
  TEST_CASE(MapperBuilder_Build_NoTemplate_Nominal)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr ButtonMapper kTestElementMapper(EButton::B15);
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(buttonA), ELEMENT_MAP_INDEX_OF(triggerLT)};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(
          true ==
          builder.SetBlueprintElementMapper(
              kMapperName, controllerElement, kTestElementMapper.Clone()));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);
    TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());
    VerifyElementMapMatchesSpec(kControllerElements, kTestElementMapper, mapper->ElementMap());
  }

  // Verifies that a trivial mapper without a template but that is marked invalid fails to build.
  TEST_CASE(MapperBuilder_Build_NoTemplate_MarkInvalid)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
    TEST_ASSERT(true == builder.InvalidateBlueprint(kMapperName));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr == mapper);
  }

  // Verifies that a mapper without a template and with elements marked for removal can be built and
  // registered, the result being an empty element map.
  TEST_CASE(MapperBuilder_Build_NoTemplate_EmptyAfterElementsRemoved)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(buttonA), ELEMENT_MAP_INDEX_OF(triggerLT)};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(
          true == builder.SetBlueprintElementMapper(kMapperName, controllerElement, nullptr));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);
    TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());
    VerifyElementMapIsEmpty(mapper->ElementMap());
  }

  // Verifies that a mapper with a template and no modification can be built and registered.
  // After build is completed, checks that the element mappers all match.
  // For this test the template is a known and documented mapper.
  TEST_CASE(MapperBuilder_Build_Template_NoModifications)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";

    const Mapper* const kTemplateMapper = Mapper::GetByName(L"StandardGamepad");
    TEST_ASSERT(nullptr != kTemplateMapper);

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kTemplateMapper->GetName()));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);
    TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());

    VerifyElementMapsAreEquivalent(mapper->ElementMap(), kTemplateMapper->ElementMap());
  }

  // Verifies that a mapper with a template and some changes applied can be built and registered, in
  // this case the changes being element modification. After build is completed, checks that the
  // element mappers all match. For this test the template is a known and documented mapper, and the
  // changes involve switching the triggers to use button 15.
  TEST_CASE(MapperBuilder_Build_Template_WithModification)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr ButtonMapper kTestElementMapper(EButton::B15);
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(triggerLT), ELEMENT_MAP_INDEX_OF(triggerRT)};

    const Mapper* const kTemplateMapper = Mapper::GetByName(L"StandardGamepad");
    TEST_ASSERT(nullptr != kTemplateMapper);

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kTemplateMapper->GetName()));

    Mapper::UElementMap expectedElementMap = kTemplateMapper->CloneElementMap();

    for (auto controllerElement : kControllerElements)
    {
      expectedElementMap.all[controllerElement] = kTestElementMapper.Clone();
      TEST_ASSERT(
          true ==
          builder.SetBlueprintElementMapper(
              kMapperName, controllerElement, kTestElementMapper.Clone()));
    }

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);
    TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());

    const Mapper::UElementMap& actualElementMap = mapper->ElementMap();
    VerifyElementMapsAreEquivalent(actualElementMap, expectedElementMap);
  }

  // Verifies that a mapper with a template and some changes applied can be built and registered, in
  // this case the changes being element removal. After build is completed, checks that the element
  // mappers all match. For this test the template is a known and documented mapper, and the changes
  // involve removing the POV.
  TEST_CASE(MapperBuilder_Build_Template_WithRemoval)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(dpadUp),
        ELEMENT_MAP_INDEX_OF(dpadDown),
        ELEMENT_MAP_INDEX_OF(dpadLeft),
        ELEMENT_MAP_INDEX_OF(dpadRight)};

    const Mapper* const kTemplateMapper = Mapper::GetByName(L"StandardGamepad");
    TEST_ASSERT(nullptr != kTemplateMapper);

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kTemplateMapper->GetName()));

    Mapper::UElementMap expectedElementMap = kTemplateMapper->CloneElementMap();

    for (auto controllerElement : kControllerElements)
    {
      expectedElementMap.all[controllerElement] = nullptr;
      TEST_ASSERT(
          true == builder.SetBlueprintElementMapper(kMapperName, controllerElement, nullptr));
    }

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);
    TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());

    const Mapper::UElementMap& actualElementMap = mapper->ElementMap();
    VerifyElementMapsAreEquivalent(actualElementMap, expectedElementMap);
  }

  // Verifies that a mapper with a template and no modification can be built and registered.
  // In this test there are changes applied but then cleared before mapper object build.
  // After build is completed, checks that the element mappers all match.
  // For this test the template is a known and documented mapper.
  TEST_CASE(MapperBuilder_Build_Template_WithClearedModifications)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    const std::set<int> kControllerElements = {
        ELEMENT_MAP_INDEX_OF(dpadUp),
        ELEMENT_MAP_INDEX_OF(dpadDown),
        ELEMENT_MAP_INDEX_OF(dpadLeft),
        ELEMENT_MAP_INDEX_OF(dpadRight)};

    const Mapper* const kTemplateMapper = Mapper::GetByName(L"StandardGamepad");
    TEST_ASSERT(nullptr != kTemplateMapper);

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kTemplateMapper->GetName()));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(
          true == builder.SetBlueprintElementMapper(kMapperName, controllerElement, nullptr));

    for (auto controllerElement : kControllerElements)
      TEST_ASSERT(true == builder.ClearBlueprintElementMapper(kMapperName, controllerElement));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);
    TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());

    VerifyElementMapsAreEquivalent(mapper->ElementMap(), kTemplateMapper->ElementMap());
  }

  // Verifies that a mapper fails to be built if it refers to itself as its own template.
  TEST_CASE(MapperBuilder_Build_Template_SelfReference)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kMapperName));
    TEST_ASSERT(nullptr == builder.Build(kMapperName));
  }

  // Verifies that a mapper fails to be built if it refers to an unkown mapper as its template.
  TEST_CASE(MapperBuilder_Build_Template_InvalidReference)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr std::wstring_view kTemplateName = L"UnknownMapper";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kTemplateName));
    TEST_ASSERT(nullptr == builder.Build(kMapperName));
  }

  // Verifies that mapper build succeeds in the presence of an acyclic chain of template
  // dependencies.
  TEST_CASE(MapperBuilder_Build_Template_Chain)
  {
    constexpr std::wstring_view kMapperNames[] = {
        L"TestMapperTemplateChainA",
        L"TestMapperTemplateChainB",
        L"TestMapperTemplateChainC",
        L"TestMapperTemplateChainD",
        L"TestMapperTemplateChainE",
        L"TestMapperTemplateChainF",
        L"TestMapperTemplateChainG"};

    MapperBuilder builder;

    for (int i = 0; i < _countof(kMapperNames); ++i)
    {
      const int nameIndex = i;
      const int templateIndex = i + 1;

      TEST_ASSERT(true == builder.CreateBlueprint(kMapperNames[nameIndex]));

      if (templateIndex < _countof(kMapperNames))
        TEST_ASSERT(
            true ==
            builder.SetBlueprintTemplate(kMapperNames[nameIndex], kMapperNames[templateIndex]));
    }

    TEST_ASSERT(nullptr != builder.Build(kMapperNames[0]));
    for (auto kMapperName : kMapperNames)
    {
      TEST_ASSERT(true == Mapper::IsMapperNameKnown(kMapperName));
      delete Mapper::GetByName(kMapperName);
    }
  }

  // Verifies that a dependent mapper fails to build if its template has been invalidated.
  TEST_CASE(MapperBuilder_Build_Template_MarkInvalid)
  {
    constexpr std::wstring_view kMapperNames[] = {L"TestMapperA", L"TestMapperB"};

    MapperBuilder builder;

    for (auto kMapperName : kMapperNames)
      TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperNames[0], kMapperNames[1]));
    TEST_ASSERT(true == builder.InvalidateBlueprint(kMapperNames[1]));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperNames[0]));
    TEST_ASSERT(nullptr == mapper);
  }

  // Verifies that mapper build succeeds in the presence of an acyclic forking chain of template
  // dependencies.
  TEST_CASE(MapperBuilder_Build_Template_Fork)
  {
    constexpr std::wstring_view kMapperNameCommonDependency = L"TestMapperTemplateForkCommonDep";
    constexpr std::wstring_view kMapperNames[] = {L"TestMapperA", L"TestMapperB"};

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperNameCommonDependency));

    for (auto kMapperName : kMapperNames)
    {
      TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));
      TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kMapperNameCommonDependency));
    }

    for (auto kMapperName : kMapperNames)
    {
      std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
      TEST_ASSERT(nullptr != mapper);
      TEST_ASSERT(Mapper::GetByName(kMapperName) == mapper.get());
    }

    TEST_ASSERT(true == Mapper::IsMapperNameKnown(kMapperNameCommonDependency));
    delete Mapper::GetByName(kMapperNameCommonDependency);
  }

  // Verifies that mapper build fails if there is a cycle in the template dependence graph.
  TEST_CASE(MapperBuilder_Build_Template_Cycle)
  {
    constexpr std::wstring_view kMapperNames[] = {
        L"TestMapperA",
        L"TestMapperB",
        L"TestMapperC",
        L"TestMapperD",
        L"TestMapperE",
        L"TestMapperF",
        L"TestMapperG"};

    MapperBuilder builder;

    for (int i = 0; i < _countof(kMapperNames); ++i)
    {
      const int nameIndex = i;
      const int templateIndex = (i + 1) % _countof(kMapperNames);

      TEST_ASSERT(true == builder.CreateBlueprint(kMapperNames[nameIndex]));
      TEST_ASSERT(
          true ==
          builder.SetBlueprintTemplate(kMapperNames[nameIndex], kMapperNames[templateIndex]));
    }

    for (int i = 0; i < _countof(kMapperNames); ++i)
      TEST_ASSERT(nullptr == builder.Build(kMapperNames[i]));
  }

  // Verifies that a mapper is built using the default force feedback actuator map if not using a
  // template and no changes are specified.
  TEST_CASE(MapperBuilder_Build_ForceFeedback_Default)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);

    const Mapper::UForceFeedbackActuatorMap& expectedActuatorMap =
        Mapper::kDefaultForceFeedbackActuatorMap;
    const Mapper::UForceFeedbackActuatorMap& actualActuatorMap =
        mapper->GetForceFeedbackActuatorMap();
    VerifyForceFeedbackActuatorMapsAreEquivalent(actualActuatorMap, expectedActuatorMap);
  }

  // Verifies that a mapper's force feedback actuator map is built completely from scratch without
  // any default actuators if no template is used and a change to the actuator map is specified.
  TEST_CASE(MapperBuilder_Build_ForceFeedback_FromScratch)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    constexpr SActuatorElement kActuatorElement = {
        .isPresent = true,
        .mode = EActuatorMode::SingleAxis,
        .singleAxis = {.axis = EAxis::Z, .direction = EAxisDirection::Negative}
    };
    TEST_ASSERT(
        true ==
        builder.SetBlueprintForceFeedbackActuator(
            kMapperName, FFACTUATOR_MAP_INDEX_OF(leftImpulseTrigger), kActuatorElement));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);

    const Mapper::UForceFeedbackActuatorMap expectedActuatorMap(
        {.leftImpulseTrigger = kActuatorElement});
    const Mapper::UForceFeedbackActuatorMap& actualActuatorMap =
        mapper->GetForceFeedbackActuatorMap();
    VerifyForceFeedbackActuatorMapsAreEquivalent(actualActuatorMap, expectedActuatorMap);
  }

  // Verifies that a mapper's force feedback actuator map is built in combination with a template's
  // actuator map if a template is specified.
  TEST_CASE(MapperBuilder_Build_ForceFeedback_WithTemplate)
  {
    constexpr std::wstring_view kMapperName = L"TestMapper";
    constexpr std::wstring_view kTemplateMapperName = L"StandardGamepad";

    MapperBuilder builder;
    TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

    constexpr SActuatorElement kActuatorElement = {
        .isPresent = true,
        .mode = EActuatorMode::SingleAxis,
        .singleAxis = {.axis = EAxis::Z, .direction = EAxisDirection::Negative}
    };
    TEST_ASSERT(
        true ==
        builder.SetBlueprintForceFeedbackActuator(
            kMapperName, FFACTUATOR_MAP_INDEX_OF(leftImpulseTrigger), kActuatorElement));
    TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, kTemplateMapperName));

    std::unique_ptr<const Mapper> mapper(builder.Build(kMapperName));
    TEST_ASSERT(nullptr != mapper);

    Mapper::UForceFeedbackActuatorMap expectedActuatorMap =
        Mapper::GetByName(kTemplateMapperName)->GetForceFeedbackActuatorMap();
    expectedActuatorMap.named.leftImpulseTrigger = kActuatorElement;

    const Mapper::UForceFeedbackActuatorMap& actualActuatorMap =
        mapper->GetForceFeedbackActuatorMap();
    VerifyForceFeedbackActuatorMapsAreEquivalent(actualActuatorMap, expectedActuatorMap);
  }
} // namespace XidiTest
