class MemnesiaStudyConfig:
    def __init__(self):
        self.experiment = {
            'name': 'Kripke (16^3 Zones)',
            'arch': 'An Arch',
            'mpi': 'An MPI',
            'mpi_exec_opts': '',
            'app': 'Kripke',
            'app_exec': '/home/samuel/devel/parallel-motifs/'
                        'structured-grids/kripke-1.1/build/kripke',
            'app_exec_opts': '--procs {},{},{} --niter 50'
        }

        self.numpes = self.gen_numpes()
        self.numpe_runcmd = self.gen_runcmds()

    def gen_numpes(self):
        '''
        Lulesh only wants cubed numpes, so match that.
        '''
        return [x**3 for x in range(1, 7)]

    def get_proc_xyz(self, numpe):
        numpe_proc_xyz = {
            1: [1, 1, 1],
            8: [2, 2, 2],
            27: [3, 3, 3],
            64: [4, 4, 4],
            125: [5, 5, 5],
            216: [6, 6, 6],
        }

        return numpe_proc_xyz[numpe]

    def gen_runcmds(self):
        res = {}
        app_exec = self.experiment['app_exec']
        app_exec_opts = self.experiment['app_exec_opts']
        mpi_exec_opts = self.experiment['mpi_exec_opts']
        for n in self.numpes:
            pxyz = self.get_proc_xyz(n)
            res[n] = 'mpirun -n {} {} {} {}'.format(
                         n,
                         mpi_exec_opts,
                         app_exec,
                         app_exec_opts.format(pxyz[0], pxyz[1], pxyz[2])
                     )
        return res

    def emit_config(self, to):
        to.write('# Run Configuration\n')
        for k, v in sorted(self.experiment.iteritems()):
            to.write('# {}: {}\n'.format(k, v))
