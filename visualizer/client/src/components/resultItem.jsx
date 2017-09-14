
import React from 'react';
import PropTypes from 'prop-types';
import classnames from 'classnames';
import Card, { CardContent, CardActions } from 'material-ui/Card';
import Collapse from 'material-ui/transitions/Collapse';
import Divider from 'material-ui/Divider';
import Typography from 'material-ui/Typography';
import ExpandMoreIcon from 'material-ui-icons/ExpandMore';
import IconButton from 'material-ui/IconButton';
import { withStyles } from 'material-ui/styles';

const styles = theme => ({
  card: {
    color: theme.palette.common.fullWhite,
    backgroundColor: theme.palette.grey[700],
    marginTop: 10,
    marginBottom: 10,
  },
  cardTitle: {
    paddingLeft: 12,
  },
  expand: {
    transform: 'rotate(0deg)',
    transition: theme.transitions.create('transform', {
      duration: theme.transitions.duration.shortest,
    }),
  },
  expandOpen: {
    transform: 'rotate(180deg)',
  },
  flexGrow: {
    flex: '1 1 auto',
  },
});

/**
 * This class encapsulate all the components relative to a single result
 * such as the summary of the warnings,the source code ofthe function etc...
 */
class ResultItem extends React.Component {
  constructor() {
    super();
    // at the begining we want all the cards to be NOT expanded
    this.state = {
      expanded: false,
    };
  }

  /**
   * Method that handle when the user clicks on the icon to expand the card
   */
  handleExpandClick = () => {
    this.setState({ expanded: !this.state.expanded });
  }

  render() {
    const classes = this.props.classes;
    return (
      <Card className={classes.card}>
        <CardActions disableActionSpacing>
          <Typography type="subheading" className={classes.cardTitle}>
            {this.props.functionName}
          </Typography>
          <div className={classes.flexGrow} />
          <IconButton
            className={classnames(classes.expand, {
              [classes.expandOpen]: this.state.expanded,
            })}
            onClick={this.handleExpandClick}
            aria-expanded={this.state.expanded}
            aria-label="Show more"
          >
            <ExpandMoreIcon />
          </IconButton>
        </CardActions>
        <Collapse in={this.state.expanded} transitionDuration="auto" unmountOnExit>
          <Divider />
          <CardContent>
            <Typography paragraph type="body2">
                Method:
            </Typography>
            <Typography paragraph>
                Heat 1/2 cup of the broth in a pot until simmering, add saffron and set aside for 10
                minutes.
            </Typography>
            <Typography paragraph>
                Heat oil in a (14- to 16-inch) paella pan or a large, deep skillet over medium-high
                heat. Add chicken, shrimp and chorizo, and cook, stirring occasionally until lightly
                browned, 6 to 8 minutes. Transfer shrimp to a large plate and set aside, leaving
                chicken and chorizo in the pan. Add pimentón, bay leaves, garlic, tomatoes, onion,
                salt and pepper, and cook, stirring often until thickened and fragrant, about 10
                minutes. Add saffron broth and remaining 4 1/2 cups chicken broth; bring to a boil.
            </Typography>
            <Typography paragraph>
                Add rice and stir very gently to distribute. Top with artichokes and peppers, and
                cook without stirring, until most of the liquid is absorbed, 15 to 18 minutes.
                Reduce heat to medium-low, add reserved shrimp and mussels, tucking them down into
                the rice, and cook again without stirring, until mussels have opened and rice is
                just tender, 5 to 7 minutes more. (Discard any mussels that don’t open.)
            </Typography>
            <Typography>
                Set aside off of the heat to let rest for 10 minutes, and then serve.
            </Typography>
          </CardContent>
        </Collapse>
      </Card>
    );
  }
}

ResultItem.propTypes = {
  classes: PropTypes.object.isRequired,
  functionName: PropTypes.string.isRequired,
};

export default withStyles(styles)(ResultItem);
