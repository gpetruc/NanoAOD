#!/usr/bin/env python
import json, re, os
import ROOT

class JSONFileFilter:
    def __init__(self,fname):
        self.keep = {}
        for _run,lumis in json.load(open(fname, 'r')).iteritems():
            run = long(_run)
            if run not in self.keep: self.keep[run] = []
            self.keep[run] += lumis
    def filterRunLumi(self,run,lumi):
        try:
            for (l1,l2) in self.keep[run]:
                if l1 <= lumi and lumi <= l2: return True
            return False
        except KeyError:
            return False
    def filterRunOnly(self,run):
        return (run in self.keep)

def _runIt(args):
    
    fname,outdir,suffix,drops,keeps,cutstring,selector,justcount = args

    infile = ROOT.TFile(fname)
    intree = infile.Get('Events')
    lumitree = infile.Get('LuminosityBlocks')
    runtree = infile.Get('Runs')

    for drop in drops: intree.SetBranchStatus(drop,0)
    for keep in keeps: intree.SetBranchStatus(keep,1)

    if fname[-5:]!='.root': raise RuntimeError
    if not justcount:
        outfile = ROOT.TFile('%s/%s%s.root'%(outdir,fname.split('/')[-1][:-5],suffix),'recreate','',9)
        outfile.cd()

    jsonelist = None
    if selector:
        intree.SetBranchStatus('run',1)
        intree.SetBranchStatus('luminosityBlock',1)
        jsonelist = ROOT.TEntryList('jsonelist','jsonelist')
        n = long(0)
        for ev in intree:
            if selector.filterRunLumi(ev.run,ev.luminosityBlock): jsonelist.Enter(n)
            n += 1
        jsonelist.SetDirectory(0)
        intree.SetEntryList(jsonelist)

    if justcount:
        print 'Would select %d entries from %s'%(intree.GetEntries(cutstring),fname)
        return
    else:
        outtree = intree.CopyTree(cutstring)
        if not selector:
            outlumitree = lumitree.CloneTree()
            outruntree = runtree.CloneTree()
        else:
            outlumitree = lumitree.CloneTree(0)
            for ev in lumitree:
                if selector.filterRunLumi(ev.run,ev.luminosityBlock): outlumitree.Fill()
            outruntree = runtree.CloneTree(0)
            for ev in runtree:
                if selector.filterRunOnly(ev.run): outruntree.Fill()

    outtree.Write()
    print 'Selected %d entries from %s'%(outtree.GetEntries(),fname)
    outlumitree.Write()
    outruntree.Write()

if __name__ == "__main__":
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] cuts.txt outputDir inputFile1 [inputFile2 inputFile3 ...]")
    parser.add_option("-s", "--suffix",dest="suffix", type="string", default='_Skim', help="Suffix which will be appended to the file name")
    parser.add_option("-D", "--drop",  dest="drop", type="string", default=[], action="append",  help="Branch to drop, passed to TTree::SetBranchStatus") 
    parser.add_option("-K", "--keep",  dest="keep", type="string", default=[], action="append",  help="Branch to keep, passed to TTree::SetBranchStatus") 
    parser.add_option("--DF", "--dropfile",  dest="dropfile", type="string", default=[], action="append",  help="File containing branches to drop, passed to TTree::SetBranchStatus") 
    parser.add_option("--KF", "--keepfile",  dest="keepfile", type="string", default=[], action="append",  help="File containing branches to keep, passed to TTree::SetBranchStatus") 
    parser.add_option("-j", "--jobs",  dest="jobs", type="int", default=0, help="Use N threads when running on multiple files");
    parser.add_option("--json",        dest="json", type="string", default=None, help="Select events using this JSON file")
    parser.add_option("--justcount",   dest="justcount", default=False, action="store_true",  help="Just report the number of selected events") 
    (options, args) = parser.parse_args()

    cuts = []
    cutfile = open(args[0], "r")
    if not cutfile: raise RuntimeError, 'Impossible to open cutfile %s'%args[0]
    for line in cutfile:
        try:
            line = line.strip()
            if len(line) == 0 or line[0] == '#': continue
            line = re.sub(r"#.*","",line)
            while line[-1] == "\\":
                line = line[:-1] + " " + file.next().strip()
                line = re.sub(r"#.*","",line)
            name,cut = [x.strip() for x in line.split(':')]
            cuts.append((name,cut))
        except ValueError, e:
            print "Error parsing cut line [%s]" % line.strip()
            raise
    cutstring = ' && '.join(['(%s)'%cut for name,cut in cuts])

    selector = JSONFileFilter(options.json) if options.json else None

    drops = options.drop[:]
    for f in options.dropfile:
        for line in open(f,'r'):
            line = re.sub(r"#.*","",line.strip())
            if len(line) == 0 or line[0] == '#': continue
            drops.append(line)
    keeps = options.keep[:]
    for f in options.keepfile:
        for line in open(f,'r'):
            line = re.sub(r"#.*","",line.strip())
            if len(line) == 0 or line[0] == '#': continue
            keeps.append(line)

    outdir = args[1]
    print "Will write selected trees to "+outdir
    if not options.justcount:
        if not os.path.exists(outdir):
            os.system("mkdir -p "+outdir)

    tasks = []
    for fname in args[2:]:
        tasks.append((fname,outdir,options.suffix,drops,keeps,cutstring,selector,options.justcount))
    if options.jobs == 0: 
        map(_runIt, tasks)
    else:
        from multiprocessing import Pool
        Pool(options.jobs).map(_runIt, tasks)
