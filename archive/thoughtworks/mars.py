class Header(object):
    _heading_seq = ['E', 'S', 'W', 'N']
    _heading_map = {
        'E' : 0,
        'S' : 1,
        'W' : 2,
        'N' : 3
    }
    _delta_map = {
        'E' : (1, 0),
        'S' : (0, -1),
        'W' : (-1, 0),
        'N' : (0, 1)
    }

    def __init__(self, heading):
        self.heading = heading


    def turn_left(self):
        self.heading = self._heading_seq[(self._heading_map[self.heading] - 1) % 4]


    def turn_right(self):
        self.heading = self._heading_seq[(self._heading_map[self.heading] + 1) % 4]

    def delta(self):
        return self._delta_map[self.heading]

class Rover(object):
    def __init__(self, xupper, yupper, x, y, heading):
        self.xupper = xupper
        self.yupper = yupper
        self.x = x
        self.y = y
        self.header = Header(heading)
        self.commands_action_map = {
            'L' : self.header.turn_left,
            'R' : self.header.turn_right,
            'M' : self.move_forward
        }


    def do(self, commands):
        for c in commands:
            action = self.commands_action_map[c]
            action()


    def move_forward(self):
        xdelta, ydelta = self.header.delta()


        self.x = (self.x + xdelta) if (self.x + xdelta in range(self.xupper + 1)) else self.x
        self.y = (self.y + ydelta) if (self.y + ydelta in range(self.yupper + 1)) else self.y

    def pos(self):
        return self.x, self.y, self.header.heading

