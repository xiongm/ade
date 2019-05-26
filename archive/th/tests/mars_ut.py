import pytest
import sys

from mars import (
    Rover,
    Header
)



def test_header():
    header = Header('E')
    header.turn_left()
    assert 'N' == header.heading
    header.turn_right()
    assert 'E' == header.heading
    header.turn_right()
    assert 'S' == header.heading

@pytest.mark.parametrize(
    'init_pos, commands, expected_pos', [
        (
            (1, 2, 'N'),
            "LMLMLMLMM",
            (1, 3, 'N')
        ),
        (
            (3, 3, 'E'),
            "MMRMMRMRRM",
            (5, 1, 'E')
        ),
        (
            (0, 0, 'W'),
            "M",
            (0, 0, 'W')
        ),
        (
            (4, 4, 'E'),
            "M",
            (5, 4, 'E')
        ),
        (
            (5, 5, 'E'),
            "M",
            (5, 5, 'E')
        ),

    ]
)
def test_basic(init_pos, commands, expected_pos):
    x, y, heading = init_pos
    rover = Rover(5, 5, x, y, heading)
    rover.do(commands)
    assert rover.pos()  == expected_pos


if __name__ == "__main__":
    sys.exit(pytest.main(['--show-capture=stdout', '-vvsx', __file__]))
