# Racing Kings Chess AI

<div align="center">
  <img src="assets/rkchess-screenshot.png" alt="Racing Kings Chess Screenshot" />
</div>

This is a racing kings chess ai. I was building it in a university project a few years ago. It was originally written in c. Now i added a cpp gui around it to make it usable again. A good expanation for racing kings chess is given at [lichess.org](https://lichess.org/variant/racingKings).

## AI Techniques

Here a list of the used techniques:

- Bitboards
- Alpha-Beta Search
- iterative depth search
- Evaluation Table based on material value and positional values
- Transposition Tables
- Quiescence Search
- start game tables
- MTD(f)

For a detailed description view i uploaded an excerpt from our project report into the repository. Here is the [Link](report.pdf)

## Usage

Clone the project via the following command.

`git clone --recurse-submodules https://github.com/Tobiasloch/rkchess-minimax-ai`

The project contains submodules. Therefore you have to use `--recurse-submodules`.

The project uses the SFML library for the gui. You need to install it before you can compile the code. For a tutorial on this vist [sfml-dev.org](https://www.sfml-dev.org/tutorials/2.6/). On debian based systems you can simply run the following command:

`sudo apt-get install libsfml-dev`

Run `make` to compile the code. The binary is then in `./bin/rkchess`. To get information on how to use it run `./bin/rkchess --help`. The tests are in the binary `./bin/test`
