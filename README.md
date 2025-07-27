# Minesweeper AI
An implementation of the AI that plays the Minesweeper game.

# How does it work?

## Basic idea

On each AI move we need to deduce a new sentence based on premises (sentences) from knowledge base of the AI.

## More deeply

### Knowledge representation

Each sentence in the knowledge base is represented by a set of cells and a number of mines among these cells.

#### Example
```
{A, B, C, D, E} = 3
```

So here we have 5 cells that we don't know anything about just yet (of course, each cell is stored as a pair of *x*, *y* coordinates) but we know that there are 3 mines among them.

### Inferencing

There are 3 possible situations from which we can infer new sentences.

#### 1. All mines

If we have a sentence where a number of cells is equal to a number of mines among these cells then we can infer that all cells are known to be mines. Example: `{A, B, C} = 3`.

#### 2. No mines

However if we have a sentence where a number of mines is 0 then we can infer that all cells are not mines. Example: `{A, B, C, D} = 0`. 

#### 3. Subtraction rule

Consider the following example:

```
+---+
|111|
|BCD|
|A2E|
+---+
```

Here we know that `{A, B, C, D, E} = 2`,
on the other side we know that `{B, C, D} = 1`. Now we can deduce a new sentence by removing all common cells from the first set and also let's subtract numbers of mines so we get `{A, E} = 1` and that's indeed right!

So the algorithm is the following:
1) Check if the second set is a subset of the first set,
2) Perform a subtraction on both sets and mines counters.

# Build

## Desktop

1) Install [premake5](https://premake.github.io/) or use one from `Vendor/Bin/Premake5`,
2) Run `./Vendor/Bin/Premake5 --help` and choose one of the supported platforms to generate a build configuration.

## Web

1) Install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html),
2) You can use `embuild.py` to build the game and `emrun.py` to run a server. Located at `Build/Scripts`.

# Assets issue

If the application runs but the screen is black then check a path to the `Assets` folder.

# [Online demo](https://defini7.github.io/demos/minesweeperai/)
