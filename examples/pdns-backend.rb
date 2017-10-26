
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
    {"name"=>"ns.example.jp","type"=>"A","content"=>"127.0.0.1",},
    {"name"=>"com.example.jp","type"=>"ALIAS","content"=>"www.example.com",},
  ]

  records.each{|rec|
    if( rec["name"] == Powerdns::Request::domain && (Powerdns::Request::type == "ANY" || rec["type"] == Powerdns::Request::type) )
      Powerdns::answer.push( rec )
    end
  }

end
