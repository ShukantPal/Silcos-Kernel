///
/// @file IRQCallbacks.h
/// @module HAL (kernel.silcos.hal)
///
/// This file imports all the __irqCallback functions written in assembly
/// and is used only in the IA32_HAL build.
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///
#ifndef HAL_SRC_IA32_IRQCALLBACKS_H__
#define HAL_SRC_IA32_IRQCALLBACKS_H__

#include <TYPE.h>

//! Function name of __irqCallback given its vector
#define __irqCallback(vt) __irqCallback##vt()

// This block imports the 160 device-IRQs present in IRQCallbacks.asm to this
// C file. These handlers from vector 32-191 inclusive are mapped into the IDT
// during boot up.
extern "C" void __irqCallback(32);
extern "C" void __irqCallback(33);
extern "C" void __irqCallback(34);
extern "C" void __irqCallback(35);
extern "C" void __irqCallback(36);
extern "C" void __irqCallback(37);
extern "C" void __irqCallback(38);
extern "C" void __irqCallback(39);
extern "C" void __irqCallback(40);
extern "C" void __irqCallback(41);
extern "C" void __irqCallback(42);
extern "C" void __irqCallback(43);
extern "C" void __irqCallback(44);
extern "C" void __irqCallback(45);
extern "C" void __irqCallback(46);
extern "C" void __irqCallback(47);
extern "C" void __irqCallback(48);
extern "C" void __irqCallback(49);
extern "C" void __irqCallback(50);
extern "C" void __irqCallback(51);
extern "C" void __irqCallback(52);
extern "C" void __irqCallback(53);
extern "C" void __irqCallback(54);
extern "C" void __irqCallback(55);
extern "C" void __irqCallback(56);
extern "C" void __irqCallback(57);
extern "C" void __irqCallback(58);
extern "C" void __irqCallback(59);
extern "C" void __irqCallback(60);
extern "C" void __irqCallback(61);
extern "C" void __irqCallback(62);
extern "C" void __irqCallback(63);
extern "C" void __irqCallback(64);
extern "C" void __irqCallback(65);
extern "C" void __irqCallback(66);
extern "C" void __irqCallback(67);
extern "C" void __irqCallback(68);
extern "C" void __irqCallback(69);
extern "C" void __irqCallback(70);
extern "C" void __irqCallback(71);
extern "C" void __irqCallback(72);
extern "C" void __irqCallback(73);
extern "C" void __irqCallback(74);
extern "C" void __irqCallback(75);
extern "C" void __irqCallback(76);
extern "C" void __irqCallback(77);
extern "C" void __irqCallback(78);
extern "C" void __irqCallback(79);
extern "C" void __irqCallback(80);
extern "C" void __irqCallback(81);
extern "C" void __irqCallback(82);
extern "C" void __irqCallback(83);
extern "C" void __irqCallback(84);
extern "C" void __irqCallback(85);
extern "C" void __irqCallback(86);
extern "C" void __irqCallback(87);
extern "C" void __irqCallback(88);
extern "C" void __irqCallback(89);
extern "C" void __irqCallback(90);
extern "C" void __irqCallback(91);
extern "C" void __irqCallback(92);
extern "C" void __irqCallback(93);
extern "C" void __irqCallback(94);
extern "C" void __irqCallback(95);
extern "C" void __irqCallback(96);
extern "C" void __irqCallback(97);
extern "C" void __irqCallback(98);
extern "C" void __irqCallback(99);
extern "C" void __irqCallback(100);
extern "C" void __irqCallback(101);
extern "C" void __irqCallback(102);
extern "C" void __irqCallback(103);
extern "C" void __irqCallback(104);
extern "C" void __irqCallback(105);
extern "C" void __irqCallback(106);
extern "C" void __irqCallback(107);
extern "C" void __irqCallback(108);
extern "C" void __irqCallback(109);
extern "C" void __irqCallback(110);
extern "C" void __irqCallback(111);
extern "C" void __irqCallback(112);
extern "C" void __irqCallback(113);
extern "C" void __irqCallback(114);
extern "C" void __irqCallback(115);
extern "C" void __irqCallback(116);
extern "C" void __irqCallback(117);
extern "C" void __irqCallback(120);
extern "C" void __irqCallback(121);
extern "C" void __irqCallback(122);
extern "C" void __irqCallback(123);
extern "C" void __irqCallback(124);
extern "C" void __irqCallback(125);
extern "C" void __irqCallback(126);
extern "C" void __irqCallback(127);
extern "C" void __irqCallback(128);
extern "C" void __irqCallback(129);
extern "C" void __irqCallback(130);
extern "C" void __irqCallback(131);
extern "C" void __irqCallback(132);
extern "C" void __irqCallback(133);
extern "C" void __irqCallback(134);
extern "C" void __irqCallback(135);
extern "C" void __irqCallback(136);
extern "C" void __irqCallback(137);
extern "C" void __irqCallback(138);
extern "C" void __irqCallback(139);
extern "C" void __irqCallback(140);
extern "C" void __irqCallback(141);
extern "C" void __irqCallback(142);
extern "C" void __irqCallback(143);
extern "C" void __irqCallback(144);
extern "C" void __irqCallback(145);
extern "C" void __irqCallback(146);
extern "C" void __irqCallback(147);
extern "C" void __irqCallback(148);
extern "C" void __irqCallback(149);
extern "C" void __irqCallback(150);
extern "C" void __irqCallback(151);
extern "C" void __irqCallback(152);
extern "C" void __irqCallback(153);
extern "C" void __irqCallback(154);
extern "C" void __irqCallback(155);
extern "C" void __irqCallback(156);
extern "C" void __irqCallback(157);
extern "C" void __irqCallback(158);
extern "C" void __irqCallback(159);
extern "C" void __irqCallback(160);
extern "C" void __irqCallback(161);
extern "C" void __irqCallback(162);
extern "C" void __irqCallback(163);
extern "C" void __irqCallback(164);
extern "C" void __irqCallback(165);
extern "C" void __irqCallback(166);
extern "C" void __irqCallback(167);
extern "C" void __irqCallback(168);
extern "C" void __irqCallback(169);
extern "C" void __irqCallback(170);
extern "C" void __irqCallback(171);
extern "C" void __irqCallback(172);
extern "C" void __irqCallback(173);
extern "C" void __irqCallback(174);
extern "C" void __irqCallback(175);
extern "C" void __irqCallback(176);
extern "C" void __irqCallback(177);
extern "C" void __irqCallback(178);
extern "C" void __irqCallback(179);
extern "C" void __irqCallback(180);
extern "C" void __irqCallback(181);
extern "C" void __irqCallback(182);
extern "C" void __irqCallback(183);
extern "C" void __irqCallback(184);
extern "C" void __irqCallback(185);
extern "C" void __irqCallback(186);
extern "C" void __irqCallback(187);
extern "C" void __irqCallback(188);
extern "C" void __irqCallback(190);
extern "C" void __irqCallback(191);

#endif/* IRQCallbacks.h */
