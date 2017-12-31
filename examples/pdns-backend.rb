
$mmdb    = MaxMindDB.new "/tmp/GeoLite2-City.mmdb"

def init
  # some initial process
end

def lookup

  if false
    p "Powerdns::Request::remote_addr=#{Powerdns::Request::remote_addr}"
    p "Powerdns::Request::type       =#{Powerdns::Request::type}"
    p "Powerdns::Request::domain     =#{Powerdns::Request::domain}"
  end

  remote_addr="#{Powerdns::Request::remote_addr.dup}"
  cc = ""
  begin
    $mmdb.lookup_string remote_addr
    cc = $mmdb.country_code
  rescue => e
    p e
    cc = "--"
  end
  #p "request-> #{cc} #{Powerdns::Request::remote_addr.dup} #{Powerdns::Request::type.dup} #{Powerdns::Request::domain.dup}"

  Powerdns::answer.clear

  records = [
    {"name"=>"example.jp","type"=>"SOA","content"=>"ns.example.jp. hostmaster.example.jp. 1 1800 900 604800 3600",},
    {"name"=>"example.jp","type"=>"NS" ,"content"=>"ns.example.jp",},
    {"name"=>"example.jp","type"=>"TXT","content"=>"COMMENT: Powerdns::Request::remote_addr=#{remote_addr} #{cc}",},
    {'name'=>'example.jp','type'=>'MX','content'=>'10 ns.example.jp',},
    {"name"=>"example.jp","type"=>"A","content"=>"192.168.0.123",},
    {"name"=>"r.example.jp","type"=>"TXT","content"=>"COMMENT: Powerdns::Request::real_remote_addr=#{Powerdns::Request::real_remote_addr}",},
    {"name"=>"ns.example.jp","type"=>"A","content"=>"127.0.0.1",},
    {"name"=>"blog.example.jp","type"=>"ALIAS","content"=>"blog.rhykw.net.",},
  ]

  records.each do |rec|
    next unless rec['name'] == Powerdns::Request.domain
    if Powerdns::Request.type == 'ANY' ||
       Powerdns::Request.type == rec['type']
      Powerdns.answer.push(rec)
    end
  end

end
