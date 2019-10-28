#include<items/potion.hpp>

#include<game.hpp>

#include<stdexcept>

std::string Potion::getName() const {
	if (g_game.isPotionKnown(id)) {
		switch (g_game.getPotionTypes()[id]->effect) {
			case Potion::Heal: return "a potion of healing";
			case Potion::Invisibility: return "a potion of invisibility";
			case Potion::Teleport: return "a potion of teleport";
			case Potion::None: return "a potion of... Water?";
			case Potion::Blindness: return "a potion of blindness";
			default: throw std::logic_error("Unknown potion effect");
		}
	} else {
	    return name;
	}
}

