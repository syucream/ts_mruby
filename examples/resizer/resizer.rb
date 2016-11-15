# parse parameters
params = {}
req = ATS::Request.new
req.args.split('&').each do |arg|
  key, value = arg.split('=')
  params[key] = value
end

# set resizing logic
resizer = nil
if params.has_key?('scale')
  resizer = Proc.new do |scale, body|
    img = Mrmagick::ImageList.new
    new_img = img.scale(scale)
    new_img.from_blob(body)
    new_img.to_blob
  end.curry.(params['scale'].to_f)
end

# set filter if params is valid
if (resizer)
  filter = ATS::Filter.new
  filter.transform! resizer
else
  ATS::rputs 'Parameters are invalid ...'
  ATS::return ATS::HTTP_BAD_REQUEST
end
