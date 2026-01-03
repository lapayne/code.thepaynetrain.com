---
sidebar_position: 2
id: first-electronics-circuit-overview
title: Your first Electronics circuit overview
description: Understand what you're working with.
slug: /bf/understand-your-first-circuit
---

## What you'll need

1. **Breadboard** (Any size)
2. **LED** (Any color)
3. **Resistor** (220 &Omega; or 330 &Omega;) - &Omega; is the symbol for ohm the measurement unit for resistance)
4. **Jumper Wires**
5. **Power Supply** (Power module or ESP32)

## 1. Understanding the Breadboard

A breadboard allows you to prototype circuits without soldering. It is divided into three main sections:

### The Power Rails

The **+ (Positive)** and **- (Negative/Ground)** rows run vertically along the sides.

- **How they work:** All holes in a single rail are connected together. Powering one hole powers the entire strip.
  ![Breadboard power and ground](/img/basic-tutorials/1/breadboard-power.png)

### The Terminal Strips (Main Board)

The main area is labeled with **Letters (A-J)** and **Numbers (1-30+)**.

- **How they work:** Unlike the power rails, these are connected **horizontally** in rows. Holes A, B, C, D, and E of Row 1 are all connected to each other, but Row 1 is _not_ connected to Row 2.
  ![Breadboard main section](/img/basic-tutorials/1/breadboard-main.png)

### The Center Channel (The Divider)

The gap in the middle separates the two sides of the terminal strips.

- **Purpose:** It allows you to plug in Integrated Circuits (ICs) or microcontrollers (like your ESP32) without shorting the pins on opposite sides.
  ![Breadboard breaker section](/img/basic-tutorials/1/breadboard-breaker.png)

---

## 2. The Components

### The LED (Light Emitting Diode)

LEDs only allow electricity to flow in **one direction**.

- **Anode (Long Leg):** Positive (+). Connects to power.
- **Cathode (Short Leg):** Negative (-). Connects to ground.
- **Flat Edge:** The plastic casing usually has a flat side on the Cathode (short leg) side.
  ![LED layout](/img/basic-tutorials/1/led.png)

### The Resistors

Resistors limit the flow of electricity to protect your LED from burning out. For this project, use either a **220 &Omega;** or **330 &Omega;** resistor.

| Value           | 4-Band Code                 | 5-Band Code                              | Image                                      |
| :-------------- | :-------------------------- | :--------------------------------------- | :----------------------------------------- |
| **220 &Omega;** | Red, Red, Brown, Gold       | Red, Red, Black, Black, Gold/Brown       | ![220 ohm](/img/basic-tutorials/1/220.jpg) |
| **330 &Omega;** | Orange, Orange, Brown, Gold | Orange, Orange, Black, Black, Gold/Brown | ![330 ohm](/img/basic-tutorials/1/330.jpg) |

> **Tip:** Resistors don't have a "direction." You can plug them in either way!

---

## 3. Why use a resistor?

Plugging an LED directly into a 5V or 3.3V power source will likely burn it out instantly. The resistor acts like a "bottleneck" in a pipe, slowing down the flow of electricity to a level the LED can safely handle.

- **Higher resistance (330 &Omega;) ** = Less light (dimmer).
- **Lower resistance (220 &Omega;) ** = More light (brighter).
