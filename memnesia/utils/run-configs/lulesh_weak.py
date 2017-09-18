class MemnesiaStudyConfig:
    def __init__(self):
        self.experiment = {
            'name': 'Lulesh (64^3 Points  per Domain)',
            'arch': 'Cray XC40',
            'mpi': 'MPICH',
            'app': 'Lulesh',
            'app_exec': '/home/samuel/devel/parallel-motifs/'
                        'unstructured-grids/lulesh/lulesh2.0',
            'app_exec_opts': '-i 10 -s 50 -p'
        }

        self.numpes = self.gen_numpes()
        self.numpe_runcmd = self.gen_runcmds()

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
