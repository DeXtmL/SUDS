﻿#include "SUDSScriptImporter.h"
#include "TestUtils.h"
PRAGMA_DISABLE_OPTIMIZATION

const FString BasicConditionalInput = R"RAWSUD(
Player: Hello
[if {x} == 1]
    NPC: Reply when x == 1
    [if {y} == 1]
        Player: Player text when x ==1 and y == 1
    [endif]
[elseif {x} == 2]
    NPC: Reply when x == 2
[else]
    NPC: Reply when x is something else
[endif]
[if {z} == true]
    Player: the end is true
[else]
    Player: the end is false
[endif]
NPC: OK
)RAWSUD";

const FString ConditionalChoiceInput = R"RAWSUD(
# First test has a regular choice first before conditional
NPC: Hello
    * First choice
        Player: I took the 1.1 choice
[if {y} == 2]
    * Second choice (conditional)
        Player: I took the 1.2 choice
    * Third choice (conditional)
        Player: I took the 1.3 choice
[else]
    * Second Alt Choice
        Player: I took the alt 1.2 choice        
[endif]
    * Common last choice
        Player: I took the 1.4 choice
# Second test has conditional choice as the first one
NPC: OK next question
[if {y} == 0]
    * First conditional choice
        Player: I took the 2.1 choice
[elseif {y} == 1]
    * Second conditional choice
        Player: I took the 2.2 choice
    [if {q} == 10]
        * Nested conditional choice
            Player: I took the 2.2.1 choice
    [endif]
[else]
    * Third conditional choice
        Player: I took the 2.3 choice
[endif]
    * Final common choice
        Player: I took the 2.4 choice
NPC: Bye
)RAWSUD";



IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestBasicConditionals,
								 "SUDSTest.TestBasicConditionals",
								 EAutomationTestFlags::EditorContext |
								 EAutomationTestFlags::ClientContext |
								 EAutomationTestFlags::ProductFilter)


bool FTestBasicConditionals::RunTest(const FString& Parameters)
{
    FSUDSScriptImporter Importer;
    TestTrue("Import should succeed", Importer.ImportFromBuffer(GetData(BasicConditionalInput), BasicConditionalInput.Len(), "BasicConditionalInput", true));

    // Test the content of the parsing
    auto NextNode = Importer.GetNode(0);
    if (!TestNotNull("Root node should exist", NextNode))
        return false;

    TestParsedText(this, "First node", NextNode, "Player", "Hello");
    TestGetParsedNextNode(this, "Get next", NextNode, Importer, false, &NextNode);

    if (TestParsedSelect(this, "First Select node", NextNode, 3))
    {
        auto SelectNode = NextNode;
        TestParsedSelectEdge(this, "First select edge 1 (if)", SelectNode, 0, "{x} == 1", Importer, &NextNode);
        TestParsedText(this, "Nested node 1", NextNode, "NPC", "Reply when x == 1");
        TestGetParsedNextNode(this, "Get next", NextNode, Importer, false, &NextNode);
        // Note: even though this is a single "if", there is an implicit "else" edge created
        if (TestParsedSelect(this, "Nested Select node", NextNode, 2))
        {
            // Nested select
            auto SelectNode2 = NextNode;
            TestParsedSelectEdge(this, "Nested select edge 1", SelectNode2, 0, "{y} == 1", Importer, &NextNode);
            TestParsedText(this, "Nested node edge 1", NextNode, "Player", "Player text when x ==1 and y == 1");
            TestGetParsedNextNode(this, "Get next", NextNode, Importer, false, &NextNode);
            if (TestParsedSelect(this, "Fallthrough select", NextNode, 2))
            {
                auto SelectNode3 = NextNode;
                TestParsedSelectEdge(this, "Final select edge 1", SelectNode3, 0, "{z} == true", Importer, &NextNode);
                TestParsedText(this, "Final select edge 1 text", NextNode, "Player", "the end is true");
                TestGetParsedNextNode(this, "Get next", NextNode, Importer, false, &NextNode);
                TestParsedText(this, "Final fallthrough", NextNode, "NPC", "OK");

                TestParsedSelectEdge(this, "Final select edge 2", SelectNode3, 1, "", Importer, &NextNode);
                TestParsedText(this, "Final select edge 2 text", NextNode, "Player", "the end is false");
                TestGetParsedNextNode(this, "Get next", NextNode, Importer, false, &NextNode);
                TestParsedText(this, "Final fallthrough", NextNode, "NPC", "OK");
                
            }

            // Go back to the nested select
            // This "else" edge should have been created automatically to fall through 
            TestParsedSelectEdge(this, "Nested select edge 2", SelectNode2, 1, "", Importer, &NextNode);
            // Just test it gets to the fallthrough, we've already tested the continuation from there
            TestParsedSelect(this, "Fallthrough select", NextNode, 2);
            
        }
        TestParsedSelectEdge(this, "First select edge 2 (elseif)", SelectNode, 1, "{x} == 2", Importer, &NextNode);
        TestParsedText(this, "Select node 2", NextNode, "NPC", "Reply when x == 2");
        TestGetParsedNextNode(this, "Get next", NextNode, Importer, false, &NextNode);
        // Just test it gets to the fallthrough, we've already tested the continuation from there
        TestParsedSelect(this, "Fallthrough select", NextNode, 2);
        
        TestParsedSelectEdge(this, "First select edge 2 (else)", SelectNode, 2, "", Importer, &NextNode);
        TestParsedText(this, "Select node 3", NextNode, "NPC", "Reply when x is something else");
        TestGetParsedNextNode(this, "Get next", NextNode, Importer, false, &NextNode);
        // Just test it gets to the fallthrough, we've already tested the continuation from there
        TestParsedSelect(this, "Fallthrough select", NextNode, 2);
    }
    
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestConditionalChoices,
                                 "SUDSTest.TestConditionalChoices",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)


bool FTestConditionalChoices::RunTest(const FString& Parameters)
{
    FSUDSScriptImporter Importer;
    TestTrue("Import should succeed", Importer.ImportFromBuffer(GetData(ConditionalChoiceInput), ConditionalChoiceInput.Len(), "ConditionalChoiceInput", true));

    // Test the content of the parsing
    auto NextNode = Importer.GetNode(0);
    if (!TestNotNull("Root node should exist", NextNode))
        return false;

    return true;
}

PRAGMA_ENABLE_OPTIMIZATION