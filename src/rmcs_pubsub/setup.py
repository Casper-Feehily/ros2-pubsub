from setuptools import find_packages, setup

package_name = 'rmcs_pubsub'

setup(
    name=package_name,
    version='0.0.1',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Casper',
    maintainer_email='casper@example.com',
    description='A minimal ROS 2 publisher and subscriber exercise.',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'publisher = rmcs_pubsub.publisher:main',
            'subscriber = rmcs_pubsub.subscriber:main',
        ],
    },
)
