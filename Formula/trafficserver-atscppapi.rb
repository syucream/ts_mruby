class TrafficserverAtscppapi < Formula
  desc "C++API of trafficserver"
  homepage "https://trafficserver.apache.org/"
  url "https://www.apache.org/dyn/closer.cgi?path=trafficserver/trafficserver-6.2.0.tar.bz2"
  mirror "https://archive.apache.org/dist/trafficserver/trafficserver-6.2.0.tar.bz2"
  sha256 "bd5e8c178d02957b89a81d1e428ee50bcca0831a6917f32408915c56f486fd85"

  bottle do
    sha256 "401991b905cf77bac76c1ef305c51e9ed8fad52c9b96510dffb6a2a45f178852" => :el_capitan
    sha256 "02b6debf85db785a781649d4533c3d1e8f05c076e21fb6000e3d9828fd0269d4" => :yosemite
    sha256 "86015b548eb9c33df0d90033e187fd765f6da5a98d2123bff7282562ad08a996" => :mavericks
  end

  head do
    url "https://github.com/apache/trafficserver.git"

    depends_on "autoconf" => :build
    depends_on "automake" => :build
    depends_on "libtool"  => :build
  end

  depends_on "trafficserver"

  needs :cxx11

  def install
    ENV.cxx11

    # Needed for correct ./configure detections
    ENV.enable_warnings

    args = %W[
      --prefix=#{prefix}
      --enable-cppapi
    ]

    system "autoreconf", "-fvi"  if build.head?
    system "./configure", *args

    # Fix wrong username in the generated startup script for bottles.
    inreplace "rc/trafficserver.in", "@pkgsysuser@", "$USER"

    system "cd lib/atscppapi/ && make && make install"
  end

end