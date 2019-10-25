//////////////////////////////////////////////////// Symbols ///////////////////////////////////////////////////////////////
/*
                                                '_'(Floor) == 1
                                                '#'(Wall) == 2
                                                '^'(Pile) == 3
*/
////////////////////////////////////////////////// Modificators ///////////////////////////////////////////////////////////
/* 1 - Nothing
                                                2 - Thorns (chance to turn damage back)
*/
//////////////////////////////////////////////////// Effects //////////////////////////////////////////////////////////////
/*
                                                1 - Map recording in Hard-mode                
                                                2 - Identify
*/
//////////////////////////////////////////// Types of weapon construction /////////////////////////////////////////////////
/*
                                        1 - One direction(Musket), scheme:

                                                @--->
                                        
                                        2 - Triple direction(Shotgun), scheme:
                                
                                              /
                                             /
                                            @--->
                                             \
                                              \

                                          !COMMENT! This isn't implemented    (lol) хуй
*/
////////////////////////////////////////////////// Tree of skills /////////////////////////////////////////////////////////
/*
                                                 Detect, is food rotten
                                                   /                \
                                        Shoot through             Chance to confuse monster
                                              |                               |
                                        Combine things             Items identify by hands(?)
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//!COMMENT! // Also it isn't needed to show to the player his satiation. And luck too. And other stuff.

#include<game.hpp>

Game g_game;

int main() {
    g_game.run();
}
