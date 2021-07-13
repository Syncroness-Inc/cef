

class CommandStatus:
    SUCCESS = 'success'


class DebugPort:
    
    def __init__(self):
        pass

    def send(self, data):
        return True

    def receive(self):
        pass