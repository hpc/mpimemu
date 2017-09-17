import os
import sys


class MemnesiaStudyConfig:
    def __init__(self):
        self.experiment = {
            'name': 'Lulesh (64^3 Points  per Domain)',
            'arch': 'Cray XC40',
            'mpi': 'MPICH',
            'app': 'Lulesh',
            'exec_path_env': 'LULESH_EXEC',
            'app_exec': '',
            'app_exec_opts': '-i 10 -s 50 -p'
        }

        self.numpes = self.gen_numpes()
        self.experiment['app_exec'] = self.get_app_exec()

        self.numpe_runcmd = self.gen_runcmds()

    def get_app_exec(self):
        exec_env = self.experiment['exec_path_env']
        execp = ''
        try:
            execp = os.environ[exec_env]
        except KeyError as e:
            print('Application exec path not set: {}'.format(e.message))
            sys.exit(os.EX_CONFIG)
        return execp

    def gen_numpes(self):
        '''
        Lulesh only wants cubed numpes.
        '''
        # return [x**3 for x in [1, 2, 3, 4, 5]]
        return [x**3 for x in [1, 2]]

    def gen_runcmds(self):
        res = {}
        app_exec = self.experiment['app_exec']
        app_exec_opts = self.experiment['app_exec_opts']
        for n in self.numpes:
            res[n] = 'mpirun -n {} {} {}'.format(n, app_exec, app_exec_opts)
        return res

    def emit_config(self, to):
        to.write('# Run Configuration\n')
        for k, v in sorted(self.experiment.iteritems()):
            to.write('# {}: {}\n'.format(k, v))
