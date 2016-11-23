class TsMruby < Formula
  desc "trafficserver's mruby extension"
  homepage "https://github.com/syucream/ts_mruby"
  sha256 "7a0208e183fcbe1259dc3ced9c3b28da5a93bf912dfeddb200d39657c73d9ff9"

  bottle do
    cellar :any
  end

  head do
    url "https://github.com/syucream/ts_mruby.git"

    depends_on "autoconf" => :build
    depends_on "automake" => :build
    depends_on "libtool"  => :build
  end

  depends_on "trafficserver" => "7.0.0+"
  needs :cxx11

  def install
    ENV.cxx11

    # Needed for correct ./configure detections
    ENV.enable_warnings

    # build mruby myself
    system "git submodule init && git submodule update"
    system "cd mruby && CFLAGS=\"-fPIC\" ./minirake"

    mruby_root = Dir.pwd + '/mruby'
    args = %W[
      --with-ts-prefix-root=/usr/local/
      --with-mruby-root=#{mruby_root}
    ]

    system "autoreconf", "-fvi" if build.head?
    system "./configure", *args
    system "make"
    system "make", "install"
    libexec.install "ts_mruby.so"
  end

end
