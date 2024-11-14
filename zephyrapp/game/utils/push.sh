if git push
then
	python3 utils/version.py push
	python3 utils/version.py tell
else
	python3 utils/version.py tell
fi


