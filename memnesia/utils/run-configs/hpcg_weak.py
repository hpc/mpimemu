class MemnesiaStudyConfig:
    def __init__(self):
        self.experiment = {
            'name': 'HPCG (96^3)',
            'arch': 'An Arch',
            'mpi': 'An MPI',
            'mpi_exec_opts': '',
            'app': 'HPCG',
            'app_exec': '/home/samuel/devel/parallel-motifs/'
                        'sparse-linear-algebra/hpcg-3.0/bin/xhpcg',
            'app_exec_opts': '--nx=96 --ny=96 --nz=96 --rt=1'
        }

        self.numpes = self.gen_numpes()
        self.numpe_runcmd = self.gen_runcmds()

    def gen_numpes(self):
        '''
        Lulesh only wants cubed numpes, so match that.
        '''
        return [x**3 for x in range(1, 7)]

    def gen_runcmds(self):
        res = {}
        app_exec = self.experiment['app_exec']
        app_exec_opts = self.experiment['app_exec_opts']
        mpi_exec_opts = self.experiment['mpi_exec_opts']
        for n in self.numpes:
            res[n] = 'mpirun -n {} {} {} {}'.format(
                         n,
                         mpi_exec_opts,
                         app_exec,
                         app_exec_opts
                     )
        return res

    def emit_config(self, to):
        to.write('# Run Configuration\n')
        for k, v in sorted(self.experiment.iteritems()):
            to.write('# {}: {}\n'.format(k, v))
