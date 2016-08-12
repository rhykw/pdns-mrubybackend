
$db_path = "/usr/share/GeoIP/GeoIPCity.dat"
$geoip   = GeoIP.new $db_path

def init
  # some initial process
end

def lookup

  if( 0 == 1)
    p "Powerdns::Request::remote_addr=#{Powerdns::Request::remote_addr}"
    p "Powerdns::Request::type       =#{Powerdns::Request::type}"
    p "Powerdns::Request::domain     =#{Powerdns::Request::domain}"
  end

  cc = ""
  begin
    $geoip.record_by_addr "#{Powerdns::Request::remote_addr}"
    cc = $geoip.country_code
  rescue => e
    cc = "--"
  end
  #p "request-> #{cc} #{Powerdns::Request::remote_addr.dup} #{Powerdns::Request::type.dup} #{Powerdns::Request::domain.dup}"

  Powerdns::answer.clear

  records = [
    {"name"=>"example.jp","type"=>"SOA","content"=>"ns.example.jp. hostmaster.example.jp. 1 1800 900 604800 3600",},
    {"name"=>"example.jp","type"=>"NS" ,"content"=>"ns.example.jp",},
    {"name"=>"example.jp","type"=>"TXT","content"=>"COMMENT: Powerdns::Request::remote_addr=#{Powerdns::Request::remote_addr.dup} #{cc}",},
    {"name"=>"ns.example.jp","type"=>"A","content"=>"127.0.0.1",},
  ]
    records.each{|rec|
      if( rec["name"] == Powerdns::Request::domain && (Powerdns::Request::type == "ANY" || rec["type"] == Powerdns::Request::type) )
        Powerdns::answer.push( rec )
      end
    }

end
