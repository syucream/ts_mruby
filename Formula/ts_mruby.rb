class TsMruby < Formula
  desc "trafficserver's mruby extension"
  homepage "https://github.com/syucream/ts_mruby"
  sha256 "19e85931acc4d4927ed53c5f6ac9df17d57f91ccc702dd629afa670c9569b7c6"

  bottle do
    cellar :any
  end

  head do
    url "https://github.com/syucream/ts_mruby.git"

    depends_on "autoconf" => :build
    depends_on "automake" => :build
    depends_on "libtool"  => :build
  end

  depends_on "trafficserver"
  depends_on "trafficserver-atscppapi" => :optional
  # depends_on "mruby"

  needs :cxx11

  def install
    ENV.cxx11

    # Needed for correct ./configure detections
    ENV.enable_warnings

    # build mruby myself
    system "git clone --depth 1 https://github.com/mruby/mruby.git"
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