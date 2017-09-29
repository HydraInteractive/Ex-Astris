#pragma once
#include <hydra/abilities/buffHandler.h>
#include <iostream>

BuffHandler::BuffHandler(){

}

BuffHandler::~BuffHandler(){

}

void BuffHandler::getActiveBuffs() {
	for (size_t i = 0; i < activeBuffs.size(); i++){
		switch (activeBuffs[i]){
		case HEALING:
			break;
		case DAMAGE:
			break;
		default:
			break;
		}
	}
}
void BuffHandler::getActiveDebuffs() {
	for (size_t i = 0; i < activeBuffs.size(); i++) {
		switch (activeDebuffs[i]) {
		case POISON:
			break;
		case BURNING:
			break;
		default:
			break;
		}
	}
}