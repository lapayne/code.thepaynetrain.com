---
sidebar_position: 2
id: first-electronics-circuit-build
title: Your first Electronics circuit build
description: Build and understand your first circuit.
slug: /basics/build-your-first-circuit
---

Now that you understand the components, it’s time to put them together. We will build a simple circuit that connects an LED to power via a resistor.

## The Circuit Diagram

Before we touch the breadboard, look at this schematic. It shows the flow of electricity: from the **Power Source &rarr; Resistor &rarr; LED &rarr; Ground**.

---

## Step-by-Step Assembly

Follow these steps exactly to ensure your circuit works and your components stay safe.

### 1. Add the Resistor

- Take your 220 &Omega; or 330 &Omega; resistor.
- Pick a spot in the middle of your breadboard across the gap.
- put the resistor with one leg on either side of the gap.
- Place one end in hole **A10** (this connects it to the LED's long leg).
- Place the other end into any hole in the **Red (+) Power Rail**.

### 2. Connect the LED

- Push the **Long Leg (Anode)** into hole **E10**.
- Push the **Short Leg (Cathode)** into hole **E11**.

### 3. Connect the Ground Wire

- Take a jumper wire.
- Plug one end into hole **A11** (this connects it to the LED's short leg).
- Plug the other end into any hole in the **Blue (-) Ground Rail**.

---

## 4. Powering the Board

If you are using an **ESP32** to power your board:

1. Connect a jumper wire from the **3V3** (or Vin) pin on the ESP32 to the **Red (+) Rail**.
2. Connect a jumper wire from a **GND** pin on the ESP32 to the **Blue (-) Rail**.

:::caution Check your connections
Double-check that the **short leg** of the LED is the one connected to the **Blue (-) Rail**. If the LED is backward, it won't light up, but it won't break either!
:::

## What should happen?

Once you plug in your ESP32 or Power Module, the LED should glow steadily.

- **Not lighting up?** Try flipping the LED around.
- **Still nothing?** Ensure your power wires are pushed firmly into the breadboard rails.
