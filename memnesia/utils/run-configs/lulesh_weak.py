class MemnesiaStudyConfig:
    def __init__(self):
        self.experiment_name = "Lulesh Weak Scaling"

    def emit_config(self, to):
        to.write('Experiment Name: {}\n'.format(self.experiment_name))
