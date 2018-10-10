#!/usr/bin/env python

import auracle_test


class TestE2EDownload(auracle_test.TestCase):

    def testDownloadSingle(self):
        p = self.Auracle(['download', 'auracle-git'])
        self.assertEqual(p.returncode, 0)
        self.assertPkgbuildExists('auracle-git')

        # We can assert ordering here because the RPC call must necessarily be
        # made prior to the tarball request.
        self.assertListEqual(self.request_uris, [
            '/rpc?type=info&v=5&arg[]=auracle-git',
            '/cgit/aur.git/snapshot/auracle-git.tar.gz'
        ])


    def testSendsDifferentAcceptEncodingHeaders(self):
        p = self.Auracle(['download', 'auracle-git'])
        self.assertEqual(p.returncode, 0)
        self.assertPkgbuildExists('auracle-git')

        accept_encoding = self.requests_sent[0].headers['accept-encoding']
        self.assertIn('deflate', accept_encoding)
        self.assertIn('gzip', accept_encoding)

        accept_encoding = self.requests_sent[1].headers['accept-encoding']
        self.assertEqual(accept_encoding, 'identity')


    def testDownloadMultiple(self):
        p = self.Auracle(['download', 'auracle-git', 'pkgfile-git'])
        self.assertEqual(p.returncode, 0)
        self.assertPkgbuildExists('auracle-git')
        self.assertPkgbuildExists('pkgfile-git')

        self.assertCountEqual(self.request_uris, [
            '/rpc?type=info&v=5&arg[]=auracle-git&arg[]=pkgfile-git',
            '/cgit/aur.git/snapshot/auracle-git.tar.gz',
            '/cgit/aur.git/snapshot/pkgfile-git.tar.gz'
        ])


    def testDownloadRecursive(self):
        p = self.Auracle(['download', '-r', 'auracle-git'])
        self.assertEqual(p.returncode, 0)
        self.assertPkgbuildExists('auracle-git')
        self.assertPkgbuildExists('nlohmann-json')

        self.assertGreater(len(self.request_uris), 2)
        self.assertIn('/rpc?type=info&v=5&arg[]=auracle-git',
                self.request_uris)
        self.assertIn('/cgit/aur.git/snapshot/auracle-git.tar.gz',
                self.request_uris)


if __name__ == '__main__':
    auracle_test.main()
