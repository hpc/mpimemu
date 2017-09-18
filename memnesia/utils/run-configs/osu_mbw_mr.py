class MemnesiaStudyConfig:
    def __init__(self):
        self.experiment = {
            'name': 'OSU Multiple Bandwidth / Message Rate Test',
            'arch': 'Some Arch',
            'mpi': 'Some MPI',
            'app': 'osu_mbw_mr',
            'app_exec': '/home/samuel/Desktop/osu-micro-benchmarks-5.3.2/'
                        'mpi/pt2pt/osu_mbw_mr',
            'app_exec_opts': ''
        }

        self.numpes = self.gen_numpes()
        self.numpe_runcmd = self.gen_runcmds()

    def gen_numpes(self):
        return [2**x for x in range(1, 5)]

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
